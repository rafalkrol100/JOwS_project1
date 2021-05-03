 #include <iostream>
 #include <fstream>
 
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/bridge-module.h"
 #include "ns3/csma-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/core-module.h"
 #include "ns3/traffic-control-module.h"
 #include "ns3/flow-monitor-module.h"
 #include "ns3/config-store.h"

  using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("mainScript");
 
 #define vssearch(loc,vec) std::find ((vec).begin (), (vec).end (), (loc)) != (vec).end ()
 
static void GenerateTraffic (Ptr<Socket> socket, Ptr<ExponentialRandomVariable> randomSize,	Ptr<ExponentialRandomVariable> randomTime)
{
    uint32_t pktSize = randomSize->GetInteger (); //Get random value for packet size
    socket->Send (Create<Packet> (pktSize));

    Time pktInterval = Seconds(randomTime->GetValue ()); //Get random value for next packet generation time 
    Simulator::Schedule (pktInterval, &GenerateTraffic, socket, randomSize, randomTime); //Schedule next packet generation
}



 int
 main (int argc, char *argv[])
 {
   // ----------------------------------------------------------------------
   // Default values for command line arguments
   // ----------------------------------------------------------------------
   bool        verbose              = true;
 
   int         simDurationSeconds   = 3;
 
   std::string pcapLocations        = "";
   uint32_t    snapLen              = PcapFile::SNAPLEN_DEFAULT;
 
   std::string csmaXLinkDataRate    = "100Mbps";
   std::string csmaXLinkDelay       = "500ns";
 
   std::string csmaYLinkDataRate    = "10Mbps";
   std::string csmaYLinkDelay       = "500ns";
 
   std::string p2pLinkDataRate      = "5Mbps";
   std::string p2pLinkDelay         = "50ms";

//   bool        enableUdpMultiSW     = true;
//   bool        enableUdpSingleSW    = true;
//   uint16_t    udpEchoPort          = 9; 
   double      lambda               = 300;
   double      mu                   = 330;

   // ----------------------------------------------------------------------
   // Create command line options and get them
   // ----------------------------------------------------------------------
   CommandLine cmd (__FILE__);
 
   cmd.Usage    ("NOTE: valid --pcap arguments are: 't2,t3,b2,b3,trlan,trwan,brlan,brwan'");
 
   cmd.AddValue ("verbose",      "Enable printing informational messages",        verbose);
 
   cmd.AddValue ("duration",     "Duration of simulation.",                       simDurationSeconds);
 
   cmd.AddValue ("pcap",         "Comma separated list of PCAP Locations to tap", pcapLocations);
   cmd.AddValue ("snapLen",      "PCAP packet capture length",                    snapLen);
 
   cmd.AddValue ("csmaXRate",    "CSMA X Link data rate",                         csmaXLinkDataRate);
   cmd.AddValue ("csmaXDelay",   "CSMA X Link delay",                             csmaXLinkDelay);
 
   cmd.AddValue ("csmaYRate",    "CSMA Y Link data rate",                         csmaYLinkDataRate);
   cmd.AddValue ("csmaYDelay",   "CSMA Y Link delay",                             csmaYLinkDelay);
 
   cmd.AddValue ("p2pRate",      "P2P Link data rate",                            p2pLinkDataRate);
   cmd.AddValue ("p2pDelay",     "P2P Link delay",                                p2pLinkDelay);
 
   cmd.Parse (argc, argv); 

    
   // --------------------------------------------------------------------
   // Users may find it convenient to turn on explicit debugging
   // for selected modules; the below lines suggest how to do this
   // --------------------------------------------------------------------
   if (verbose)
     {
       LogComponentEnable ("mainScript", LOG_LEVEL_INFO);
     }
 
 
   // ======================================================================
   // Define the list of valid PCAP taps
   // ----------------------------------------------------------------------
   std::vector<std::string> pcapTaps;
   pcapTaps.push_back ("t2");              // multi-switch  UDP echo client
   pcapTaps.push_back ("t3");              // single-switch UDP echo server
   pcapTaps.push_back ("b2");              // multi-switch  UDP echo server
   pcapTaps.push_back ("b3");              // single-switch UDP echo client
   pcapTaps.push_back ("trlan");           // top router    LAN side
   pcapTaps.push_back ("trwan");           // top router    WAN side
   pcapTaps.push_back ("brlan");           // bottom router LAN side
   pcapTaps.push_back ("brwan");           // bottom router WAN side
 
   // ----------------------------------------------------------------------
   // Parse the pcapLocations string into pcapLocationVec
   // ----------------------------------------------------------------------
   std::vector<std::string> pcapLocationVec;
   if (pcapLocations != "")
     {
       std::stringstream sStream (pcapLocations);
 
       while ( sStream.good () )
         {
           std::string substr;
           getline ( sStream, substr, ',' );
           if (vssearch (substr,pcapTaps))
             {
               pcapLocationVec.push_back ( substr );
             }
           else
             {
               NS_LOG_ERROR ("WARNING: Unrecognized PCAP location: <" + substr + ">");
             }
         }
 
       for (std::vector<std::string>::const_iterator
            ploc = pcapLocationVec.begin ();
            ploc  != pcapLocationVec.end ();
            ++ploc)
         {
           NS_LOG_INFO ("PCAP capture at: <" + *ploc + ">");
         }
     }
 
   // ----------------------------------------------------------------------
   // Set PCAP packet capture maximum packet length
   // ----------------------------------------------------------------------
   if (snapLen != PcapFile::SNAPLEN_DEFAULT)
     {
       Config::SetDefault ("ns3::PcapFileWrapper::CaptureSize",   UintegerValue (snapLen));
     }
 
   // ======================================================================
   // Create the nodes & links required for the topology shown in comments above.
   // ----------------------------------------------------------------------

      NS_LOG_INFO ("INFO: Create nodes.");    // - - - - - - - - - - - - - - - -
                                           // Node IP     : Description
                                           // - - - - - - - - - - - - - - - -
   Ptr<Node> t2  = CreateObject<Node> ();  // 192.168.1.2 : Top multi-switch udp echo client
   Ptr<Node> t3  = CreateObject<Node> ();  // 192.168.1.3 : Top single-switch   udp echo server
                                           //             :
   Ptr<Node> ts1 = CreateObject<Node> ();  // <no IP>     : Top switch #1 (bridge)
                                           //             :
   Ptr<Node> tr  = CreateObject<Node> ();  // 192.168.1.1 : Router connecting top LAN & WAN
                                           // 76.1.1.1    :
                                           //             :
   Ptr<Node> br  = CreateObject<Node> ();  // 76.1.1.2    : Router connecting WAN & bottom LANs
                                           // 192.168.2.1 :
                                           //             :
   Ptr<Node> bs1 = CreateObject<Node> ();  // <no IP>     : Bottom switch #1 (bridge)
                                           //             :
   Ptr<Node> b2  = CreateObject<Node> ();  // 192.168.2.2 : Bottom multi-switch udp echo server
 
   Ptr<Node> b3  = CreateObject<Node> ();  // 192.168.2.3 : Bottom single-switch   udp echo client
                                           // - - - - - - - - - - - - - - - -
 
   // ----------------------------------------------------------------------
   // Give the nodes names
   // ----------------------------------------------------------------------
   Names::Add ("t2",  t2);
   Names::Add ("t3",  t3);
   Names::Add ("ts1", ts1);
   Names::Add ("tr",  tr);
   Names::Add ("br",  br);
   Names::Add ("bs1", bs1);
   Names::Add ("b2",  b2);
   Names::Add ("b3",  b3);

   // ======================================================================
   // Create CSMA links to use for connecting LAN nodes together
   // ----------------------------------------------------------------------
 
   // ----------------------------------------
   // CSMA [X]
   // ----------------------------------------
   NS_LOG_INFO ("L2: Create a " <<
                csmaXLinkDataRate << " " <<
                csmaXLinkDelay << " CSMA link for csmaX for LANs.");
   CsmaHelper csmaX;
   csmaX.SetChannelAttribute ("DataRate", StringValue (csmaXLinkDataRate));
   csmaX.SetChannelAttribute ("Delay",    StringValue (csmaXLinkDelay));
 
   // ----------------------------------------
   // CSMA [Y]
   // ----------------------------------------
   NS_LOG_INFO ("L2: Create a " <<
                csmaYLinkDataRate << " " <<
                csmaYLinkDelay << " CSMA link for csmaY for LANs.");
   CsmaHelper csmaY;
   csmaY.SetChannelAttribute ("DataRate", StringValue (csmaYLinkDataRate));
   csmaY.SetChannelAttribute ("Delay",    StringValue (csmaYLinkDelay));
 
   // ----------------------------------------------------------------------
   // Now, connect the top LAN nodes together with csma links.
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L2: Connect nodes on top LAN together with half-duplex CSMA links.");
   
   // Switch top LAN chain: t2-ts1-tr
   NetDeviceContainer link_t2_ts1   = csmaX.Install (NodeContainer (t2,  ts1));

   // Single-switch top LAN link: t3-ts1-tr
   NetDeviceContainer link_t3_ts1   = csmaX.Install (NodeContainer (t3,  ts1));

   // Common link for top LAN between ts1 and tr (for t2 and t3 to get to tr)
   NetDeviceContainer link_tr_ts1   = csmaY.Install (NodeContainer (tr,  ts1));
   // ----------------------------------------------------------------------
   // And repeat above steps to connect the bottom LAN nodes together
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L2: Connect nodes on bottom LAN together with half-duplex CSMA links.");

   // Switch bottom LAN chain: b2-bs1-br
   NetDeviceContainer link_b2_bs1   = csmaY.Install (NodeContainer (b2,  bs1));

   // Single-switch bottom LAN link: b3-bs1-br
   NetDeviceContainer link_b3_bs1   = csmaY.Install (NodeContainer (b3,  bs1));

   // Common link for bottom LAN between bs1 and br (for b2 and b3 to get to br)
   NetDeviceContainer link_br_bs1   = csmaX.Install (NodeContainer (br,  bs1));

   // ======================================================================
   // Create a point-to-point link for connecting WAN nodes together
   // (this type of link is full-duplex)
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L2: Create a " <<
                p2pLinkDataRate << " " <<
                p2pLinkDelay    << " Point-to-Point link for the WAN.");
 
   PointToPointHelper p2p;
   p2p.SetDeviceAttribute  ("DataRate", StringValue (p2pLinkDataRate));
   p2p.SetChannelAttribute ("Delay",    StringValue (p2pLinkDelay));
 
   // ----------------------------------------------------------------------
   // Now, connect top router to bottom router with a p2p WAN link
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L2: Connect the routers together with the Point-to-Point WAN link.");

    
   NetDeviceContainer link_tr_br;
   link_tr_br = p2p.Install (NodeContainer (tr,br));
 
   // ======================================================================
   // Manually create the list of NetDevices for each switch
   // ----------------------------------------------------------------------
 
   // Top Switch 1 NetDevices
   NetDeviceContainer ts1nd;
   ts1nd.Add (link_t2_ts1.Get (1));
   ts1nd.Add (link_t3_ts1.Get (1));
   ts1nd.Add (link_tr_ts1.Get (1));

   // Bottom Switch 1 NetDevices
   NetDeviceContainer bs1nd;
   bs1nd.Add (link_br_bs1.Get (1));
   bs1nd.Add (link_b2_bs1.Get (1));
   bs1nd.Add (link_b3_bs1.Get (1));

   // ======================================================================
   // Install bridging code on each switch
   // ----------------------------------------------------------------------
   BridgeHelper bridge;

   bridge.Install (ts1, ts1nd);
   
   bridge.Install (bs1, bs1nd);
   
    
   // ======================================================================
   // Install the L3 internet stack  (TCP/IP)
   // ----------------------------------------------------------------------
   InternetStackHelper ns3IpStack;
 
   // ----------------------------------------------------------------------
   // Install the L3 internet stack on UDP endpoints
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L3: Install the ns3 IP stack on udp client and server nodes.");
   NodeContainer endpointNodes (t2, t3,  b2, b3);
   ns3IpStack.Install (endpointNodes);
 
   // ----------------------------------------------------------------------
   // Install the L3 internet stack on routers.
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L3: Install the ns3 IP stack on routers.");
   NodeContainer routerNodes (tr, br);
   ns3IpStack.Install (routerNodes);
 
   // ====================================================================== 
   // Assign top LAN IP addresses
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L3: Assign top LAN IP Addresses.");

   
   NetDeviceContainer topLanIpDevices;        // - - - - - -- - - - - - -
   topLanIpDevices.Add (link_tr_ts1.Get (0));  // NOTE: order matters here
   topLanIpDevices.Add (link_t2_ts1.Get (0));  //       for IP address
   topLanIpDevices.Add (link_t3_ts1.Get (0));  //       assignment
                                               // - - - - - -- - - - - - -
   Ipv4AddressHelper ipv4;
   ipv4.SetBase ("192.168.1.0", "255.255.255.0");
   Ipv4InterfaceContainer interfacestop = ipv4.Assign  (topLanIpDevices);
   
   // ----------------------------------------------------------------------
   // Assign bottom LAN IP addresses
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L3: Assign bottom LAN IP Addresses.");
 
   NetDeviceContainer botLanIpDevices;        // - - - - - -- - - - - - -
   botLanIpDevices.Add (link_br_bs1.Get (0));  // NOTE: order matters here
   botLanIpDevices.Add (link_b2_bs1.Get (0));  //       for IP address
   botLanIpDevices.Add (link_b3_bs1.Get (0));  //       assignment
                                               // - - - - - -- - - - - - -
 
   ipv4.SetBase ("192.168.2.0", "255.255.255.0");
   Ipv4InterfaceContainer interfacesbot = ipv4.Assign  (botLanIpDevices);

   // ----------------------------------------------------------------------
   // Assign WAN IP addresses
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L3: Assign WAN IP Addresses.");
 
   ipv4.SetBase ("76.1.1.0", "255.255.255.0");
   ipv4.Assign  (link_tr_br);

   
   // ======================================================================
   // Calculate and populate routing tables
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("L3: Populate routing tables.");
   
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      NS_LOG_INFO ("check");
    
  //application content
    
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    Ptr<Socket> recvSink = Socket::CreateSocket (endpointNodes.Get(0), tid);
    InetSocketAddress local = InetSocketAddress (interfacestop.GetAddress (1), 80);
    recvSink->Bind (local);

    Ptr<Socket> source = Socket::CreateSocket (endpointNodes.Get(1), tid);
    InetSocketAddress remote = InetSocketAddress (interfacestop.GetAddress (1), 80);
    source->Connect (remote);

    double mean = 1.0/lambda;
    Ptr<ExponentialRandomVariable> randomTime = CreateObject<ExponentialRandomVariable> ();
    randomTime->SetAttribute ("Mean", DoubleValue (mean));

    mean = (1000000.0/(8*mu)-30); // (1 000 000 [b/s])/(8 [b/B] * packet service rate [1/s]) - 30 [B (header bytes)]
    Ptr<ExponentialRandomVariable> randomSize = CreateObject<ExponentialRandomVariable> ();
    randomSize->SetAttribute ("Mean", DoubleValue (mean));

    Simulator::ScheduleWithContext (source->GetNode ()->GetId (), Seconds (1.0), &GenerateTraffic, source, randomSize, randomTime);


 
 
   // ======================================================================
   // Print routing tables at T=0.1
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // NOTE: Node 0 and Node 13 must have non-empty tables (except for local 
   //       loopback and local LAN) if routing is operating correctly.
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("Set up to print routing tables at T=0.1s");
 
   Ptr<OutputStreamWrapper> routingStream =
     Create<OutputStreamWrapper> ("global-routing-multi-switch-plus-router.routes", std::ios::out);
 
   Ipv4GlobalRoutingHelper g;
   g.PrintRoutingTableAllAt (Seconds (0.1), routingStream);
 
 
   // ======================================================================
   // Configure PCAP traces
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("Configure PCAP Tracing (if any configured).");
 
   // - - - - - - - - - - - - - -
   // multi-switch UDP echo client
   // - - - - - - - - - - - - - -
   if (vssearch ("t2",pcapLocationVec))
     {
       csmaX.EnablePcap ("t2.pcap",    topLanIpDevices.Get (1), true, true);
     }
 
   // - - - - - - - - - - - - - -
   // multi-switch UDP echo server
   // - - - - - - - - - - - - - -
   if (vssearch ("b2",pcapLocationVec))
     {
       csmaY.EnablePcap ("b2.pcap",    botLanIpDevices.Get (1), true, true);
     }
 
   // - - - - - - - - - - - - - -
   // single-switch UDP echo client
   // - - - - - - - - - - - - - -
   if (vssearch ("b3",pcapLocationVec))
     {
       csmaY.EnablePcap ("b3.pcap",    botLanIpDevices.Get (2), true, true);
     }
      
   // - - - - - - - - - - - - - -
   // single-switch UDP echo server
   // - - - - - - - - - - - - - -
   if (vssearch ("t3",pcapLocationVec))
     {
       csmaX.EnablePcap ("t3.pcap",    topLanIpDevices.Get (2), true, true);
     }
 
   // - - - - - - - - - - - - - -
   // top router, LAN side
   // - - - - - - - - - - - - - -
   if (vssearch ("trlan",pcapLocationVec))
     {
       csmaY.EnablePcap ("trlan.pcap", topLanIpDevices.Get (0), true, true);
     }
 
   // - - - - - - - - - - - - - -
   // bottom router, LAN side
   // - - - - - - - - - - - - - -
   if (vssearch ("brlan",pcapLocationVec))
     {
       csmaX.EnablePcap ("brlan.pcap", botLanIpDevices.Get (0), true, true);
     }
 
   // - - - - - - - - - - - - - -
   // top router, WAN side
   // - - - - - - - - - - - - - -
   if (vssearch ("trwan",pcapLocationVec))
     {
       p2p.EnablePcap ("trwan.pcap",  link_tr_br.Get (0),       true, true);
     }
 
   // - - - - - - - - - - - - - -
   // bottom router, WAN side
   // - - - - - - - - - - - - - -
   if (vssearch ("brwan",pcapLocationVec))
     {
       p2p.EnablePcap ("brwan.pcap",  link_tr_br.Get (1),       true, true);
     }
  
   // ======================================================================
   // Now, do the actual simulation.
   // ----------------------------------------------------------------------
   NS_LOG_INFO ("Run Simulation for " << simDurationSeconds << " seconds.");
 
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

   Simulator::Stop (Seconds (simDurationSeconds));
   Simulator::Run ();
 
   Simulator::Destroy ();
   NS_LOG_INFO ("Done.");
 
 }