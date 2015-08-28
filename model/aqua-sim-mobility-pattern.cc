//...


#include "ns3/log.h"

#include "aqua-sim-mobility-pattern.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AquaSimMobilityPattern");
NS_OBJECT_ENSURE_REGISTERED(AquaSimMobilityPattern);


void
AquaSimPosUpdateHelper::Expire() 
{
	m_mP->HandleLocUpdate();
	if (m_posUpdateHelper.IsRunning) {
		m_posUpdateHelper.Remove();
	}
	m_posUpdateHelper.Schedule(m_mP->UptIntv());
}

/**
* @param duration trajectory during this duration time can be cached
* @param interval the time interval between two adjacent locations
* 					in the trajectory, i.e., location update interval
* @param X	   	node's X coordinate
* @param Y		node's Y coordinate
* @param Z		node's Z coordinate
*/
LocationCache::LocationCache(Time duration,
							Time interval, double X, double Y, double Z,
							double dX, double dY, double dZ) :
	locations(1 + size_t(ceil(duration / interval))),
	m_bIndex(0), m_size(1)
{
	this->interval = interval;
	locations[0].loc = Location3D(X, Y, Z);
	locations[0].sp = Speed(dX, dY, dZ);
}

/**
* @return the max number of locations can be stored
*/
inline size_t
LocationCache::Capacity() {
	return locations.size();
}

inline Time
LocationCache::LastUpdateTime() {
	return Empty() ? -1 : m_fstUptTime + m_interval * (m_size - 1);
}


bool
LocationCache::InRange(Time t) {
	return t >= m_fstUptTime && t < (NOW + m_interval * (Capacity() - 1));
}

/**
* append a new location in the cache
* the time gap between this new one and the last one in
* the cache must equal to the attribute interval
*
* @param loc the new location
*/
void
LocationCache::AddNewLoc(const LocationCacheElem &lce) {
	locations[(m_bIndex + m_size) % Capacity()] = lce;
	if (Full())
		m_bIndex++;
	else
		m_size++;
}


LocationCacheElem
LocationCache::GetLocByTime(Time t) {
	if (t < m_fstUptTime || t > LastUpdateTime()) {
		throw std::out_of_range("LocationCache::GetLocByTime");
	}

	return locations[m_bIndex + size_t((t - m_fstUptTime) / m_interval)];
}


/*****
* implementations of AquaSimMobilityPattern
*****/

AquaSimMobilityPattern::AquaSimMobilityPattern() :
m_node(NULL), m_lc(NULL), m_posUpdateHelper(this)
{
}

TypeId
AquaSimMobilityPattern::GetTypeId(void)
{
	static TypeId tid = TypeId("ns3::AquaSimMobilityPattern")
		.SetParent<Object>()
		.AddConstructor<AquaSimMobilityPattern>()
		.AddAttribute("UpdateInt", "Set the update interval. Default is 0.001.")
			TimeValue(0.001),
			GetAccessor(&AquaSimMobilityPattern::m_updateInterval),
			MakeTimeChecker<Time>())
		;
	return tid;
}


AquaSimMobilityPattern::~AquaSimMobilityPattern() {
delete m_lc;
}

void
AquaSimMobilityPattern::SetNode(AquaSimNode *n) {
	m_node = n;
}

/**
* mobility pattern starts to work, i.e., the host node starts to move
*/
void
AquaSimMobilityPattern::Start() {
	if (NULL != m_lc) {
		delete m_lc;
		m_lc = NULL;
	}

	m_lc = new LocationCache(5, m_updateInterval
		m_node->X(), m_node->Y(), m_node->Z(),
		m_node->dX(), m_node->dY(), m_node->dZ());
	Init();

	m_posUpdateHelper.Expire(NULL);
}


/**
* initiate aquasim mobility pattern, derived class
* should overload this one
*/
void AquaSimMobilityPattern::Init() {
return;
}

/**
* a dummy one
*/
LocationCacheElem
AquaSimMobilityPattern::GenNewLoc() {
	NS_LOG_FUNCTION(this >> "A Dummy one! Shouldn't call this function!");
	LocationCacheElem newLoc;
	return newLoc;
}

/*
void AquaSimMobilityPattern::updateGridKeeper()
{
//If we need this ??? new_moves involves destX, Y
//but we don't know them in many mobility patterns
if(GridKeeper::instance() != NULL){
GridKeeper::instance()->new_moves(node);
}
}
*/

/**
* the interface for AquaSimPosUpdateHelper to call
*/
void
AquaSimMobilityPattern::HandleLocUpdate() {
	LocationCacheElem e;
	while (m_lc->LastUpdateTime() < NOW) {
		e = GenNewLoc();
		RestrictLocByBound(e);
		m_lc->AddNewLoc(e);
	}
	//find the time closest one to NOW
	e = GetLocByTime(NOW);
	double oldX = m_node->X();
	m_node->X() = e.loc.X();
	m_node->Y() = e.loc.Y();
	m_node->Z() = e.loc.Z();
	m_node->dX() = e.sp.dX();
	m_node->dY() = e.sp.dY();
	m_node->dZ() = e.sp.dZ();
	m_node->speed() = e.sp.getSpeed();		//could be completed by MobilityModel::GetVelocity

	if (oldX != m_node->X())			//TODO adjust node to satisfy topography use here T()
		m_node->T()->updateNodesList(m_node, oldX); //X_ is the key value of SortList

	//updateGridKeeper(); //do I really need this?!!!!!
	m_node->PositionUpdateTime() = NOW;
	//record the movement
	//namLogMobility(m_lc->lastUpdateTime(), e);
}


LocationCacheElem
AquaSimMobilityPattern::GetLocByTime(Time t) {
	if (!m_lc->InRange(t)) {
		throw std::out_of_range("AquaSimMobilityPattern::GetLocByTime");
	}

	LocationCacheElem newLce;
	while (m_lc->LastUpdateTime() < t) {
		newLce = GenNewLoc();
		RestrictLocByBound(newLce);
		m_lc->AddNewLoc(newLce);
	}

	return m_lc->GetLocByTime(t);
}


/**
* log the position change in nam file
*	Not implemented currently
*
* @param vx velocity projected to x axis
* @param vy velocity projected to y axis
* @param interval the interval to the next update

void AquaSimMobilityPattern::namLogMobility(Time t, LocationCacheElem &lce) {
node->namlog("n -t %f -s %d -x %f -y %f -z %f -U %f -V %f -T %f",
t,
node->nodeid(),
lce.loc.X(), lce.loc.Y(), lce.loc.Z(),
lce.sp.dX(), lce.sp.dY(),
update_interval);
}
*/

/**
* update node's position. if it's out of bound
* We simply bounce the node by the corresponding edge
*/
void
AquaSimMobilityPattern::RestrictLocByBound(LocationCacheElem &lce){
	/*
	 * TODO
	 * This should all be replaced using ns3::Box Class
	 *
	 */
	bool recheck = true;
	Ptr<CubicPositionAllocator> T = m_node->T();

	while (recheck) {
		recheck = false;
		recheck = recheck || BounceByEdge(lce.loc.X(), lce.sp.dX(), T->GetMinX(), true);
		recheck = recheck || BounceByEdge(lce.loc.X(), lce.sp.dX(), T->GetMaxX(), false);
		recheck = recheck || BounceByEdge(lce.loc.Y(), lce.sp.dY(), T->GetMinY(), true);
		recheck = recheck || BounceByEdge(lce.loc.Y(), lce.sp.dY(), T->GetMaxY(), false);
		recheck = recheck || BounceByEdge(lce.loc.Z(), lce.sp.dZ(), T->GetMinZ(), true);
		recheck = recheck || BounceByEdge(lce.loc.Z(), lce.sp.dZ(), T->GetMaxZ(), false);
	}
}

/**
* bounce the node by the edge if it is out of range
*
* @param coord 1D coordinate, could be one of X, Y, Z
* @param speed speed projected to the same axis as coord
* @param bound the value of lower/upper bound
* @param lowerBound to specify if bound is lower Bound (true),
* 				otherwise, it's upper bound
*
* @return  true for coord is changed, false for not
*/
bool
AquaSimMobilityPattern::BounceByEdge(double &coord, double &dspeed,
double bound, bool lowerBound) {
	if ((lowerBound && (coord < bound)) /*below lower bound*/
		|| (!lowerBound && (coord > bound)) /*beyond upper bound*/) {
		coord = bound + bound - coord;
		dspeed = -dspeed;
		return true;
	}

	return false;
}


}  //namespace ns3
