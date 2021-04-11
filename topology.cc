 #include <iostream>
 #include <fstream>
 
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/bridge-module.h"
 #include "ns3/csma-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/internet-module.h"

  using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("GlobalRoutingMultiSwitchPlusRouter");
 
 #define vssearch(loc,vec) std::find ((vec).begin (), (vec).end (), (loc)) != (vec).end ()
 
 int
 main (int argc, char *argv[])
 {
   // ----------------------------------------------------------------------
   // Default values for command line arguments
   // ----------------------------------------------------------------------
   bool        verbose              = true;
 
   int         simDurationSeconds   = 60;
 
   bool        enableUdpMultiSW     = true;
   bool        enableUdpSingleSW    = true;
 
   std::string pcapLocations        = "";
   uint32_t    snapLen              = PcapFile::SNAPLEN_DEFAULT;
 
   std::string csmaXLinkDataRate    = "100Mbps";
   std::string csmaXLinkDelay       = "500ns";
 
   std::string csmaYLinkDataRate    = "10Mbps";
   std::string csmaYLinkDelay       = "500ns";
 
   std::string p2pLinkDataRate      = "5Mbps";
   std::string p2pLinkDelay         = "50ms";
 
   uint16_t    udpEchoPort          = 9;  // The well-known UDP echo port
 

   // ----------------------------------------------------------------------
   // Create command line options and get them
   // ----------------------------------------------------------------------
   CommandLine cmd (__FILE__);
 
   cmd.Usage    ("NOTE: valid --pcap arguments are: 't2,t3,b2,b3,trlan,trwan,brlan,brwan'");
 
   cmd.AddValue ("verbose",      "Enable printing informational messages",        verbose);
 
   cmd.AddValue ("duration",     "Duration of simulation.",                       simDurationSeconds);
 
   cmd.AddValue ("udpMultiSW",   "Enable udp over multi-switch links",            enableUdpMultiSW);
   cmd.AddValue ("udpSingleSW",  "Enable udp over single-switch links",           enableUdpSingleSW);
 
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
       LogComponentEnable ("GlobalRoutingMultiSwitchPlusRouter", LOG_LEVEL_INFO);
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

 
 
   // ======================================================================
   // Set some simulator-wide values
   // ======================================================================
 
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
   