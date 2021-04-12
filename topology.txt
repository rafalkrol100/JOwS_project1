# JOwS_project1
 // ---------------------------------------------------------------------- //  
 //                                                                        //  
 //   192.168.  192.168.                                                   //  
 //    .1.2      .1.3                                                      //  
 //  ---------  ---------                                                  //  
 //  |  t2   |  |  t3   |                                                  //  
 //  |  UDP  |  |  UDP  |                                                  //  
 //  |  echo |  |  echo |    Node t2 is a UDP echo client (multi-switch)   //  
 //  | client|  | server|    Node t3 is a UDP echo server (single-switch)  //  
 //  ---------  ---------                                                  //  
 //   CSMA(t2)   CSMA(t3)                                                  //  
 //     [X]        [X]                                                     //  
 //     [X]        [X]                                                     //  
 //     [X]        [X]        Node ts1 is L2 switch                        //  
 //     [X]        [X]        The top LAN is subnet 192.168.1.*            //  
 //     [X]        [X]                                                     //  
 //    CSMA       CSMA                                                     //  
 //   ------------------                                                   //  
 //   |  ts1 (switch)  |                                                   //  
 //   ------------------                                                   //  
 //          CSMA                                                          //  
 //           [Y]                                                          //  
 //           [Y]                                                          //  
 //          CSMA(trlan)    192.168.1.1                                    //  
 //   ------------------                                                   //  
 //   |  tr (router)   |    Node tr is an L3 router                        //  
 //   ------------------      (between 192.168.1.* & 76.1.1.*)             //  
 //           P2P(trwan)    76.1.1.1                                       //  
 //           [P]                                                          //  
 //           [P]                                                          //  
 //           [P]                                                          //  
 //           [P]                                                          //  
 //           [P]           The WAN is 76.1.1.*                            //  
 //           [P]                                                          //  
 //           [P]                                                          //  
 //           [P]                                                          //  
 //           P2P(brwan)    76.1.1.2                                       //  
 //   ------------------                                                   //  
 //   |  br (router)   |    Node br is an L3 router                        //  
 //   ------------------      (between 192.168.2.* & 76.1.1.*)             //  
 //          CSMA(brlan)    192.168.2.1                                    //  
 //           [X]                                                          //  
 //           [X]                                                          //  
 //          CSMA                                                          //  
 //   ------------------     Node bs1 is L2 switch                         //  
 //   |  bs1 (switch)  |     The bottom LAN is subnet 192.168.2.*          //  
 //   ------------------                                                   //  
 //    CSMA       CSMA                                                     //  
 //     [Y]        [Y]                                                     //  
 //     [Y]        [Y]                                                     //  
 //     [Y]        [Y]                                                     //  
 //     [Y]        [Y]                                                     //  
 //    CSMA(b2)   CSMA(b3)                                                 //  
 //  ---------  ---------                                                  //  
 //  |  b2   |  |  b3   |                                                  //  
 //  |  UDP  |  |  UDP  |                                                  //  
 //  |  echo |  |  echo |    Node b2 is a UDP echo server (multi-switch)   //  
 //  | server|  | client|    Node b3 is a UDP echo client (single-switch)  //  
 //  ---------  ---------                                                  //  
 //   192.168.  192.168.                                                   //  
 //    .2.2      .2.3                                                      //  
 //                                                                        //  
 // ---------------------------------------------------------------------- //  
