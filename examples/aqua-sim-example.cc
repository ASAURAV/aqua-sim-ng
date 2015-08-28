/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/aqua-sim-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("AquaSimExample");

Simulation::Simulation ()
  : m_txPower (1)
    m_rxPower (1)
    m_initialEnergy (10000)
    m_idlePower (0)
    m_ifqLen (50) //used?
    m_x (100)
    m_y (100)
    m_z (50)
    m_dataRate (100)  //bps
    m_seed (0)
    m_stop (600)  //simulation stop
    m_preStop (20)  //time to prepare to stop
    m_tr ("aqua-sim-example.tr")  //trace file
    m_dataFile ("aqua-sim-example.data")  //data file
    m_width (20)
    m_adj (10)
    m_interval (0.001)
    //LL settings TODO
    m_minDelay (50)
    m_delay (25)
    //Antenna settings TODO   
    //Mac
    m_bitRate (1.0e4)
    m_encodingEfficiency (1)
    m_packetSize (500)
    //Phy  TODO
    m_cpThresh (10)  //10.0
    m_csThresh (0)  //1.559e-11
    m_rxThresh (0)  //3.652e-10
    m_pt (0.2818)
    m_freq (25)  //frequency range in khz
    m_k (2.0)  //spherical spreading
{
}

int 
main (int argc, char *argv[])
{
  bool verbose = true;

  Simulation sim;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);
  cmd.AddValue ("DataRate", "Set the data rate", sim.m_dataRate);
  //TODO add additional cmd features here
  cmd.Parse (argc,argv);


  //default values for simulation
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", sim.packetSize);
  Config::SetDefault ("ns3::OnOffApplication::DataRate", sim.dataRate);


  
  NS_LOG_INFO ("Creating nodes");
  NodeContainer c;
  c.Create (3);
  NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n23 = NodeContainer (c.Get (2), c.Get (3));
  //TODO create sink here

  //create net device/channels TODO complete/update this
  PointToPointHelper p2p;  //using this for now
  NetDeviceContainer nd01 = p2p.Install (n01);
  NetDeviceContainer nd12 = p2p.Install (n12);
  NetDeviceContainer nd23 = p2p.Install (n23);

  // TODO ip addresses?? (using created class which inherites from ipv4)

  NS_LOG_INFO ("Creating On/Off Application");
  //create a typical app TODO needs to be updated
  /*PacketSocketAddress socket;  //using this for now
  socket.SetSingleDevice (
  OnOffHelper app ("ns3::UdpSocketFactory", Address (socket));
  */

  NS_LOG_INFO ("Enabling Routing");
  //TODO
  //Use a aqua-sim helper to do so
  
  //helpername created
  //helpername.SetAttribute ("DataRate", DataRateValue (m_dataRate));

  //TODO Phy layer??? uan implements in main

  Simulator::Stop (Seconds (sim.m_stop));

  NS_LOG_INFO ("Starting Simulation");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Simulation End");

  return 0;
}


