#include "RoutingProtocol.h" 
#include "ns3/uinteger.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4-l3-protocol.h"
#include "RoutingProtocol.h"
#include "ns3/packet.h"
/*static ns3::TypeId GetTypeId ()
  {
  static ns3::TypeId tid = ns3::TypeId ("ns3::dsdv::DeferredRouteOutputTag")
  .SetParent<ns3::Tag> ()
  .SetGroupName ("Dsdv")
  .AddConstructor<DeferredRouteOutputTag> ()
  ;
  return tid;
  }
  */
//******dsdv_port //EMPTY
ns3::Ptr<ns3::Socket> RoutingProtocol::findSocketWithInterfaceAddress(ns3::Ipv4InterfaceAddress interface) const{
	auto itr = socketToInterfaceMap.cbegin();
	while(itr != socketToInterfaceMap.cend()){
		if(itr->second == interface) return itr->first;
	}
	return NULL;
}
void RoutingProtocol::NotifyInterfaceDown(uint32_t interface){
	ns3::Ptr<ns3::Socket> socket = findSocketWithInterfaceAddress(ptrIp->GetAddress(interface,0));
	socket->Close();
	socketToInterfaceMap.erase(socket);
	routingTable.deleteRoutesWithInterface(ptrIp->GetAddress(interface,0));
	//Message printing for interface up
}
void RoutingProtocol::NotifyInterfaceUp(uint32_t interface){
	//Message printing for interface up
	ns3::Ipv4InterfaceAddress iface = ptrIp->GetAddress(interface,0);
	ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (), ns3::UdpSocketFactory::GetTypeId ()); 
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvDsdv,this));
	socket->BindToNetDevice (ptrIp->GetNetDevice (interface));
	socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), DSDV_PORT));
	socket->SetAllowBroadcast (true);
	socket->SetAttribute ("IpTtl",ns3::UintegerValue (1));
	socketToInterfaceMap.insert (std::make_pair (socket,iface));
	ns3::Ptr<ns3::NetDevice> dev = ptrIp->GetNetDevice (ptrIp->GetInterfaceForAddress (iface.GetLocal ()));
	/*inclomplete getlocal get net device"*/
}
void RoutingProtocol::NotifyAddAddress(uint32_t interfaceNo,ns3::Ipv4InterfaceAddress address){
	if (!ptrIp->IsUp (interfaceNo))
	{
		return;
	}
	ns3::Ipv4InterfaceAddress iface = ptrIp->GetAddress (interfaceNo,0);
	ns3::Ptr<ns3::Socket> socket = RoutingProtocol::findSocketWithInterfaceAddress (iface);
	if (!socket)
	{
		if (iface.GetLocal () == ns3::Ipv4Address ("127.0.0.1"))
		{
			return;
		}
		ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (),ns3::UdpSocketFactory::GetTypeId ());
		/*imortant 		//socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDsdv,this));     */
		socket->BindToNetDevice (ptrIp->GetNetDevice (interfaceNo));
		socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), DSDV_PORT));
		socket->SetAllowBroadcast (true);
		socketToInterfaceMap.insert (std::make_pair (socket,iface));
		ns3::Ptr<ns3::NetDevice> dev = ptrIp->GetNetDevice (ptrIp->GetInterfaceForAddress (iface.GetLocal ()));
		//important		RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (),/*seqno=*/ 0, /*iface=*/ iface,/*hops=*/ 0,
		//				/*next hop=*/ iface.GetBroadcast (), /*lifetime=*/ ns3::Simulator::GetMaximumSimulationTime ());   
		routingTable.addRouteEntry (rt);
	}
}
void RoutingProtocol::NotifyRemoveAddress(uint32_t interfaceNo, ns3::Ipv4InterfaceAddress ifaceAddress){
	ns3::Ptr<ns3::Socket> socket = findSocketWithInterfaceAddress(ifaceAddress);
	if(socket != NULL){
		socketToInterfaceMap.erase (socket);
		if (ptrIp->GetNAddresses (interfaceNo)){	
			ns3::Ipv4InterfaceAddress iface = ptrIp->GetAddress(interfaceNo,0);
			ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (),ns3::UdpSocketFactory::GetTypeId ());
			socket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvDsdv,this));
			socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), DSDV_PORT));
			socket->SetAllowBroadcast (true);
			socketToInterfaceMap.insert (std::make_pair (socket,iface));
		}
	}
}

bool RoutingProtocol::RouteInput (ns3::Ptr<const ns3:: Packet> p, const ns3::Ipv4Header &header, ns3::Ptr<const ns3::NetDevice> idev, UnicastForwardCallback ucb, MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb){
	if(socketToInterfaceMap.empty())  return false;
	ns3::Ipv4Address src=header.GetSource();
	ns3::Ipv4Address dst=header.GetDestination();
	for(auto itr =socketToInterfaceMap.begin();itr!=socketToInterfaceMap.end();itr++)
	{
		if( src == (itr->second).GetLocal())
			return true;
	}
	RoutingTableEntry rte;
	for(auto itr = socketToInterfaceMap.begin();itr != socketToInterfaceMap.end();itr++){
		ns3::Ipv4InterfaceAddress ifaceAddr = itr->second;
		if(ptrIp->GetInterfaceForAddress(ifaceAddr.GetLocal()) == ptrIp->GetInterfaceForDevice(idev)){
			if(dst == ifaceAddr.GetBroadcast() || dst.IsBroadcast()){
				ns3::Ptr<ns3::Packet> packet =  p->Copy();
				if(lcb.IsNull() == false){
					lcb(p,header,ptrIp->GetInterfaceForDevice(idev));
				}
				else{
					ecb(p,header,ns3::Socket::ERROR_NOROUTETOHOST);
				}
				if(header.GetTtl() > 1){
					RoutingTableEntry toBroadcast;
					if(routingTable.search(dst,toBroadcast)){
						ns3::Ptr<ns3::Ipv4Route> route = toBroadcast.getRoute();
						ucb(route,packet,header);
					}
					else{
						//drop packet
					}
				}
				return true;
			}
		}
	}

	RoutingTableEntry toDst;
	if (routingTable.search(dst,toDst))
	{
		RoutingTableEntry ne;
		if (routingTable.search(toDst.getNextHop(),ne))
		{
			ns3::Ptr<ns3::Ipv4Route> route = ne.getRoute ();
		}
	}
	else{ 
		//we will direct this packet for interzone processsing
	}   
}
void RoutingProtocol::DoDispose(){
	for(auto itr = socketToInterfaceMap.cbegin();itr != socketToInterfaceMap.cend();itr++){
		(itr->first)->Close();
	}
	socketToInterfaceMap.clear();
	ns3::Ipv4RoutingProtocol::DoDispose();
}
void RoutingProtocol::recvDsdv(ns3::Ptr<ns3::Socket> socket){
	ns3::Address srcAddr;
	ns3::Ptr<ns3::Packet> packet = socket->ns3::Socket::RecvFrom(srcAddr);
	if (packet == 0) {//print cannot return a next in sequence packet}
	}
	ns3::InetSocketAddress inet = ns3::InetSocketAddress::ConvertFrom(srcAddr);
	ns3::Ipv4Address sender = inet.GetIpv4();
	ns3::Ipv4Address receiver = socketToInterfaceMap[socket].GetLocal();


}

bool RoutingProtocol::RouteInput (ns3::Ptr<const ns3:: Packet> p,
                              const ns3::Ipv4Header &header,
                              ns3::Ptr<const ns3::NetDevice> idev,
                              UnicastForwardCallback ucb,
                              MulticastForwardCallback mcb,
                              LocalDeliverCallback lcb,
                              ErrorCallback ecb)
 {
      if(socketToInterfaceMap.empty())  return false;
      ns3::Ipv4Address src=header.GetSource();
      ns3::Ipv4Address dst=header.GetDestination();
      for(auto itr =socketToInterfaceMap.begin();itr!=socketToInterfaceMap.end();itr++)
      {
          if( src == (itr->second).GetLocal())
            return true;
      }
      
      RoutingTableEntry rte;
      
     
         RoutingTableEntry toDst;
   if (routingTable.search(dst,toDst))
     {
       RoutingTableEntry ne;
       if (routingTable.search(toDst.getNextHop(),ne))
         {
           ns3::Ptr<ns3::Ipv4Route> route = ne.getRoute ();
       
           ucb (route,p,header);
           return true;
         }
          else
      { //we will direct this packet for interzone processsing
      }
      
     } 
     

     
      
}
ns3::Ptr<ns3::Ipv4Route> RoutingProtocol::RouteOutput (ns3::Ptr<ns3::Packet> p, const ns3::Ipv4Header &header, ns3::Ptr<ns3::NetDevice> oif,ns3::Socket::SocketErrno &sockerr)
{
   if (!p)
     {
       return RoutingProtocol::LoopbackRoute(header,oif);
     }
     if (socketToInterfaceMap.empty ())
     {
       
       ns3::Ptr<ns3::Ipv4Route> route;
       return route;
     }
      RoutingTableEntry rt;
      ns3::Ipv4Address dst = header.GetDestination ();
      if( routingTable.search(header.GetDestination (),rt))
      {
         ns3::Ptr<ns3::Ipv4Route> route=rt.getRoute();
          if(rt.getHopsCount()==1)
            {
               
              if (oif != 0 && route->GetOutputDevice () != oif)
                  return ns3::Ptr<ns3::Ipv4Route>();
            }
            return route;
      }
      else{
             RoutingTableEntry rti;
            if(routingTable.search(rt.getNextHop(),rti))
            {
               ns3::Ptr<ns3::Ipv4Route> route=rti.getRoute();
               if (oif != 0 && route->GetOutputDevice () != oif)
               {
                 return ns3::Ptr<ns3::Ipv4Route>();
               }
               return route;
            }
             
             
            
      }
     return LoopbackRoute (header,oif);

}
void RoutingProtocol::send(ns3::Ptr<ns3::Ipv4Route>route, ns3::Ptr<const ns3::Packet>packet, const ns3::Ipv4Header &header)
{
    ns3::Ptr<ns3::Ipv4L3Protocol> l3 =RoutingProtocol::ptrIp->GetObject<ns3::Ipv4L3Protocol> ();
   NS_ASSERT (l3 != 0);
   ns3::Ptr<ns3::Packet> p = packet->Copy ();
   l3->Send (p,route->GetSource (),header.GetDestination (),header.GetProtocol (),route);
}
void RoutingProtocol::sendPeriodicUpdates()
 {
    std::map<ns3::Ipv4Address, RoutingTableEntry>allroute;
    routingTable.getAllRoutes(allroute);
    if(allroute.empty())
    {
      return;
    }
     for (std::map<ns3::Ptr<ns3::Socket>, ns3::Ipv4InterfaceAddress>::const_iterator j = socketToInterfaceMap.begin (); j
        != socketToInterfaceMap.end (); ++j)
     { ns3::Ptr<ns3::Socket> socket = j->first;
       ns3::Ipv4InterfaceAddress iface = j->second;
       ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet> ();
       DsdvHeader dsdvHeader;
          for(auto itr=allroute.begin();itr!=allroute.end();itr++)
          {
            
           if (itr->second.getHopsCount() == 0)
             {
               RoutingTableEntry ownEntry;
               dsdvHeader.set_IP (ptrIp->GetAddress (1,0).GetLocal ());
               dsdvHeader.set_seq( itr->second.getSeqNumber() + 1);
               dsdvHeader.set_hops (itr->second.getHopsCount ()+1);
               dsdvHeader.set_loca(Location(0.0,0.0));
               routingTable.search(ptrIp->GetAddress (1,0).GetBroadcast (),ownEntry);

               ownEntry.setSeqNumber(dsdvHeader.get_seq());
              routingTable.updateRoute(ownEntry);
                packet->AddHeader(dsdvHeader);

             } 
             else{
                    dsdvHeader. set_IP (itr->second.getDsptIp());
               dsdvHeader.set_seq(itr->second.getSeqNumber());
               dsdvHeader.set_hops(itr->second. getHopsCount () + 1);
                dsdvHeader.set_loca(Location(0.0,0.0));
               packet->AddHeader (dsdvHeader);
             }
          } 
          if(dsdvHeader.get_hops()<=RoutingProtocol::zoneRadius)
             {
           socket->Send (packet);
       
       ns3::Ipv4Address destination;
       if (iface.GetMask () == ns3::Ipv4Mask::GetOnes ())
         {
           destination = ns3::Ipv4Address ("255.255.255.255");
         }
       else
         {
           destination = iface.GetBroadcast ();
         }
       socket->SendTo (packet, 0, ns3::InetSocketAddress (destination, DSDV_PORT)); 
             }
        
     }        
 }
