#include "RoutingProtocol.h" 
#include "gzrppacket.h"
#include "ns3/uinteger.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4-l3-protocol.h"
#include "RoutingProtocol.h"
#include "ns3/packet.h"
//using namespace ns3 for rapid code developement, after final commit will be removed
using namespace ns3;
ns3::Ptr<ns3::Socket> RoutingProtocol::findIntrazoneSocket(ns3::Ipv4InterfaceAddress address) const{
	for(auto itr = intrazoneSocketMap.begin();itr!=intrazoneSocketMap.end();itr++){
		if(address == itr->second) return itr->first;
	}
	return ns3::Ptr<ns3::Socket>(0);
}
ns3::Ptr<ns3::Socket> RoutingProtocol::findInterzoneSocket(ns3::Ipv4InterfaceAddress address) const{
	for(auto itr = interzoneSocketMap.cbegin();itr!=interzoneSocketMap.cend();itr++){
		if(address == itr->second) return itr->first;
	}
	return ns3::Ptr<ns3::Socket>(0);
}
void RoutingProtocol::NotifyInterfaceDown(uint32_t interface){
	ns3::Ptr<ns3::Socket> intrazoneSocket = findIntrazoneSocket(ptrIp->GetAddress(interface,0));
	ns3::Ptr<ns3::Socket> interzoneSocket = findInterzoneSocket(ptrIp->GetAddress(interface,0));
	intrazoneSocket->Close();
	interzoneSocket->Close();
	intrazoneSocketMap.erase(intrazoneSocket);
	interzoneSocketMap.erase(interzoneSocket);
	routingTable.deleteRoutesWithInterface(ptrIp->GetAddress(interface,0));
	// 		To-Do
	//Message printing for interface down
	//
}
void RoutingProtocol::NotifyInterfaceUp(uint32_t interface){
	//             To-Do
	//Message Printing for interface up
	//
	ns3::Ipv4InterfaceAddress iface = ptrIp->GetAddress(interface,0);
	//discard loopback interface
	if(iface.GetLocal() == ns3::Ipv4Address("127.0.0.1")) return;
	//add intrazone socket
	ns3::Ptr<ns3::Socket> intrazoneSocket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (), ns3::UdpSocketFactory::GetTypeId ()); 
	intrazoneSocket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvUpdates,this));
	intrazoneSocket->BindToNetDevice (ptrIp->GetNetDevice (interface));
	intrazoneSocket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), INTRAZONE_PORT));
	intrazoneSocket->SetAllowBroadcast (true);
	intrazoneSocket->SetAttribute ("IpTtl",ns3::UintegerValue (1));
	//add interzone socket
	ns3::Ptr<ns3::Socket> interzoneSocket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (), ns3::UdpSocketFactory::GetTypeId ()); 
	interzoneSocket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvInterzoneControlPackets,this));
	interzoneSocket->BindToNetDevice (ptrIp->GetNetDevice (interface));
	interzoneSocket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), INTERZONE_PORT));
	interzoneSocket->SetAllowBroadcast (true);
	interzoneSocket->SetAttribute ("IpTtl",ns3::UintegerValue (64));
	interzoneSocketMap.insert (std::make_pair (interzoneSocket,iface));
	interzoneSocketMap.insert (std::make_pair (interzoneSocket,iface));
	//add entry to routing table
	ns3::Ptr<ns3::NetDevice> dev = ptrIp->GetNetDevice (ptrIp->GetInterfaceForAddress (iface.GetLocal ()));
	RoutingTableEntry rt (dev, iface.GetBroadcast (),  0,Metric(0), iface, iface.GetBroadcast (),  ns3::Simulator::GetMaximumSimulationTime ());
	routingTable.addRouteEntry (rt);
	if (nodeAddress == ns3::Ipv4Address ()){
		nodeAddress = iface.GetLocal ();
	}
}
void RoutingProtocol::NotifyAddAddress(uint32_t interfaceNo,ns3::Ipv4InterfaceAddress address){
	ns3::Ptr<ns3::Ipv4L3Protocol>  l3 = ptrIp->GetObject<ns3::Ipv4L3Protocol>();
	if(!l3->IsUp(interfaceNo)) return;
	ns3::Ipv4InterfaceAddress iface = l3->GetAddress (interfaceNo,0);
	ns3::Ptr<ns3::Socket> intrazoneSocket = RoutingProtocol::findIntrazoneSocket (iface);
	if (!intrazoneSocket){
		if (iface.GetLocal () == ns3::Ipv4Address ("127.0.0.1")){
			return;
		}
		ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (),ns3::UdpSocketFactory::GetTypeId ());
		socket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvUpdates,this));     
		socket->BindToNetDevice (ptrIp->GetNetDevice (interfaceNo));
		socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), INTRAZONE_PORT));
		socket->SetAllowBroadcast (true);
		intrazoneSocketMap.insert (std::make_pair (socket,iface));
		ns3::Ptr<ns3::NetDevice> dev = ptrIp->GetNetDevice (ptrIp->GetInterfaceForAddress (iface.GetLocal ()));
		RoutingTableEntry rt (dev, iface.GetBroadcast (), 0, Metric(0),iface, iface.GetBroadcast (),  ns3::Simulator::GetMaximumSimulationTime ()); 
		routingTable.addRouteEntry (rt);
	}
	ns3::Ptr<ns3::Socket> interzoneSocket = RoutingProtocol::findInterzoneSocket (iface);
	if (!interzoneSocket){
		if (iface.GetLocal () == ns3::Ipv4Address ("127.0.0.1")){
			return;
		}
		ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (),ns3::UdpSocketFactory::GetTypeId ());
		socket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvInterzoneControlPackets,this));     
		socket->BindToNetDevice (ptrIp->GetNetDevice (interfaceNo));
		socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), INTERZONE_PORT));
		socket->SetAllowBroadcast (true);
		interzoneSocketMap.insert (std::make_pair (socket,iface));
	}
}
void RoutingProtocol::NotifyRemoveAddress(uint32_t interfaceNo, ns3::Ipv4InterfaceAddress ifaceAddress){
	ns3::Ptr<ns3::Socket> intrazoneSocket = findIntrazoneSocket(ifaceAddress);
	if(intrazoneSocket){
		intrazoneSocketMap.erase (intrazoneSocket);
		if (ptrIp->GetNAddresses (interfaceNo)){	
			ns3::Ipv4InterfaceAddress iface = ptrIp->GetAddress(interfaceNo,0);
			ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (),ns3::UdpSocketFactory::GetTypeId ());
			socket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvUpdates,this));
			socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), INTRAZONE_PORT));
			socket->SetAllowBroadcast (true);
			intrazoneSocketMap.insert (std::make_pair (socket,iface));
		}
	}
	ns3::Ptr<ns3::Socket> interzoneSocket = findInterzoneSocket(ifaceAddress);
	if(interzoneSocket){
		interzoneSocketMap.erase (interzoneSocket);
		if (ptrIp->GetNAddresses (interfaceNo)){	
			ns3::Ipv4InterfaceAddress iface = ptrIp->GetAddress(interfaceNo,0);
			ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket (GetObject<ns3::Node> (),ns3::UdpSocketFactory::GetTypeId ());
			socket->SetRecvCallback (MakeCallback (&RoutingProtocol::recvInterzoneControlPackets,this));
			socket->Bind (ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), INTERZONE_PORT));
			socket->SetAllowBroadcast (true);
			interzoneSocketMap.insert (std::make_pair (socket,iface));
		}
	}
	
}
void RoutingProtocol::recvUpdates(ns3::Ptr<ns3::Socket> socket){
	ns3::Address sourceAddress;
	ns3::Ptr<ns3::Packet> advPacket = Create<Packet>();
	ns3::Ptr<ns3::Packet> packet = socket->RecvFrom(sourceAddress);
	ns3::InetSocketAddress inetSourceAddr = ns3::InetSocketAddress::ConvertFrom(sourceAddress);
	Ipv4Address sender = inetSourceAddr.GetIpv4();
	Ipv4Address receiver = intrazoneSocketMap[socket].GetLocal();
	Ptr<NetDevice> dev = ptrIp->GetNetDevice(ptrIp->GetInterfaceForAddress(receiver));
	routingTable.deleteAllInvalidRoutes();
	EventId event;
	uint32_t packetSize = packet->GetSize();
	while(packetSize>0){
		GzrpPacket header;
		packet->RemoveHeader(header);
		packetSize-=header.ns3::Header::GetSerializedSize();
		int count = 0;
		for(auto itr = intrazoneSocketMap.begin();itr!=intrazoneSocketMap.end();itr++){
			Ipv4InterfaceAddress interface = itr->second;
			if(header.getSrcIp() == interface.GetLocal()) count++;
		}
		if(count>0) continue;
		if(header.getZoneId() != getZoneId() && header.getSrcIp()==sender){
			routingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
			continue;
		}
		RoutingTableEntry fwdEntry,advEntry;
		bool entryFound = routingTable.search(header.getSrcIp(),fwdEntry);
		if(entryFound == false){
			if(header.getSeqNo() % 2 != 1){
				RoutingTableEntry re (dev,header.getSrcIp(),header.getSeqNo(),header.getMetric(),ptrIp->GetAddress(ptrIp->GetInterfaceForAddress(receiver),0),sender,ns3::Simulator::Now(),settlingTime,true);
				routingTable.addRouteEntry(re);
				std::set<uint32_t> neighbourSet;
				header.getNeighbourZones(neighbourSet);
				for(uint32_t zones : neighbourSet){
					routingTable.addZoneIp(header.getSrcIp(),zones);
				}
			}
		}else{
			//Adding route in advRoutingTable if not present in advRoutinTable but present in routingTable
			if(!advRoutingTable.search(header.getSrcIp(),advEntry)){
				advRoutingTable.addRouteEntry(fwdEntry);
				advRoutingTable.search(header.getSrcIp(),advEntry);
			}
			if(header.getSeqNo() %2 !=1){
				if(header.getSeqNo() > advEntry.getSeqNumber()){
					if(advRoutingTable.deleteEvent(header.getSrcIp())){
						//Print Cancelling Timer
					}
					if(header.getMetric() != advEntry.getMetric().getMagnitude()) {
						advEntry.setSeqNumber(header.getSeqNo());
						advEntry.setLifeTime(Simulator::Now());
						advEntry.setChangedState(true);
						advEntry.setNextHop(sender);
						Metric met(header.getMetric());
						advEntry.setMetric(met);
						advEntry.setSettlingTime(settlingTime);
						event = Simulator::Schedule(settlingTime,&RoutingProtocol::SendTriggeredUpdate,this);
						advRoutingTable.addEvent(header.getSrcIp(),event);
						routingTable.updateRoute(advEntry);
						advRoutingTable.updateRoute(advEntry);
					}else{
						advEntry.setSeqNumber(header.getSeqNo());
						advEntry.setLifeTime(Simulator::Now());
						advEntry.setChangedState(true);
						advEntry.setNextHop(sender);
						Metric met(header.getMetric());
						advEntry.setMetric(met);
						advRoutingTable.updateRoute(advEntry);
					}
				}else if(header.getSeqNo() == fwdEntry.getSeqNumber()){
					if(header.getMetric()<fwdEntry.getMetric().getMagnitude()){
						//print to cancel timer
						advRoutingTable.deleteEvent(header.getSrcIp());
						advEntry.setSeqNumber(header.getSeqNo());
						advEntry.setLifeTime(Simulator::Now());
						advEntry.setChangedState(true);
						advEntry.setNextHop(sender);
						Metric met(header.getMetric());
						advEntry.setMetric(met);
						advEntry.setSettlingTime(settlingTime);
						event = Simulator::Schedule(settlingTime,&RoutingProtocol::SendTriggeredUpdate,this);
						advRoutingTable.addEvent(header.getSrcIp(),event);
						routingTable.updateRoute(advEntry);
						advRoutingTable.updateRoute(advEntry);
					}else{
						if(!advRoutingTable.anyRunningEvent(header.getSrcIp())){
							//update timer because we got notified that sender(next ip) is present.
							if(advEntry.getNextHop() == sender){
								advEntry.setLifeTime(Simulator::Now());
								routingTable.updateRoute(advEntry);
							}
							advRoutingTable.deleteRouteEntry(header.getSrcIp());
						}
					}
				}else{
					if(!advRoutingTable.anyRunningEvent(header.getSrcIp())){
						advRoutingTable.deleteEvent(header.getSrcIp());
					}
				}
			}else{
				if(sender == advEntry.getNextHop()){
					std::map<Ipv4Address,RoutingTableEntry> dstsWithNextHopSrc;
					routingTable.getListOfAddressWithNextHop(header.getSrcIp(),dstsWithNextHopSrc);
					routingTable.deleteRouteEntry(header.getSrcIp());
					advEntry.setSeqNumber(header.getSeqNo());
					advEntry.setChangedState(true);
					advRoutingTable.updateRoute(advEntry);
					for(auto i = dstsWithNextHopSrc.begin();i!=dstsWithNextHopSrc.end();i++){
						i->second.setSeqNumber(i->second.getSeqNumber()+1);
						i->second.setChangedState(true);
						advRoutingTable.addRouteEntry(i->second);
						routingTable.deleteRouteEntry(i->second.getDsptIp());
					}
				}else{
					if(!advRoutingTable.anyRunningEvent(header.getSrcIp())){
						advRoutingTable.deleteRouteEntry(header.getSrcIp());
					}
				}
			}

			Simulator::Schedule(MicroSeconds(random_variable->GetInteger(0,1000)),&RoutingProtocol::SendTriggeredUpdate,this);
		}
	}
}

void RoutingProtocol::SendTriggeredUpdate(){
	//print log to send triggered update
	std::map<Ipv4Address,RoutingTableEntry> allRoutes;
	advRoutingTable.getAllRoutes(allRoutes);
	for(auto j = intrazoneSocketMap.begin();j!=intrazoneSocketMap.end();j++){
		GzrpPacket header;
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		Ptr<Packet> packet = Create<Packet>();
		for(auto i = allRoutes.begin();i!=allRoutes.end();i++){
			//print about sending packet
			RoutingTableEntry temp = i->second;
			if((i->second.isChanged())&&!(advRoutingTable.anyRunningEvent(temp.getDsptIp()))){
				header.setSrcIp(i->second.getDsptIp());
				header.setSeqNo(i->second.getSeqNumber());
				header.setMetric(i->second.getMetric().getMagnitude()+1);
				temp.setChangedState(false);
				advRoutingTable.deleteEvent(temp.getDsptIp());
				if(!(temp.getSeqNumber()%2))
					routingTable.updateRoute(temp);
				packet->AddHeader(header);
				advRoutingTable.deleteRouteEntry(temp.getDsptIp());
			}
		}
		if(packet->GetSize()>=20){
			RoutingTableEntry temp2;
			routingTable.search(ptrIp->GetAddress(1,0).GetBroadcast(),temp2);
			header.setSrcIp(ptrIp->GetAddress(1,0).GetLocal());
			header.setSeqNo(temp2.getSeqNumber());
			header.setMetric((temp2.getMetric()+1).getMagnitude());
			packet->AddHeader(header);
			Ipv4Address destination;
			if(iface.GetMask() == Ipv4Mask::GetOnes()){
				destination = Ipv4Address("255.255.255.255");
			}else{
				destination = iface.GetBroadcast();
			}
			socket->SendTo(packet,0,InetSocketAddress(destination,INTRAZONE_PORT));
		}
	}
}

void RoutingProtocol::sendPeriodicUpdates(){
	std::map<Ipv4Address,RoutingTableEntry> removedAddresses,allRoutes;
	routingTable.purge(removedAddresses);
	mergeTriggerPeriodicUpdates();
	routingTable.getAllRoutes(allRoutes);
	if(allRoutes.empty()) return;
	for(auto j = intrazoneSocketMap.begin();j!=intrazoneSocketMap.end();j++){
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		Ptr<Packet> packet = Create<Packet>();
		for(auto i = allRoutes.begin();i!=allRoutes.end();i++){
			GzrpPacket header;
			if(i->second.getMetric().getMagnitude() == 0){
				RoutingTableEntry ownEntry;
				header.setSrcIp(ptrIp->GetAddress(1,0).GetLocal());
				header.setSeqNo(i->second.getSeqNumber()+2);
				header.setMetric(i->second.getMetric().getMagnitude()+1);
				routingTable.search(ptrIp->GetAddress(1,0).GetBroadcast(),ownEntry);
				ownEntry.setSeqNumber(header.getSeqNo());
				routingTable.updateRoute(ownEntry);
				packet->AddHeader(header);
			}else{
				header.setSrcIp(i->second.getDsptIp());
				header.setSeqNo(i->second.getSeqNumber());
				header.setMetric(i->second.getMetric().getMagnitude()+1);
				packet->AddHeader(header);
			}
		}
		for(auto itr = removedAddresses.begin();itr!=removedAddresses.end();itr++){
			GzrpPacket removedHeader;
			removedHeader.setSrcIp(itr->second.getDsptIp());
			removedHeader.setSeqNo(itr->second.getSeqNumber()+1);
			removedHeader.setMetric(itr->second.getMetric().getMagnitude()+1);
			packet->AddHeader(removedHeader);
		}
		socket->Send(packet);
		Ipv4Address destination;
		if(iface.GetMask()==Ipv4Mask::GetOnes()){
			destination = Ipv4Address("255.255.255.255");
		}else{
			destination = iface.GetBroadcast();
		}
		socket->SendTo(packet,0,InetSocketAddress(destination,INTRAZONE_PORT));
	}
	updateTimer.Schedule(periodicUpdateInterval+MicroSeconds(25*random_variable->GetInteger(0,1000)));
}

void RoutingProtocol::mergeTriggerPeriodicUpdates(){
	std::map<Ipv4Address,RoutingTableEntry> allRoutes;
	advRoutingTable.getAllRoutes(allRoutes);
	if(allRoutes.size()>0){
		for(auto itr = allRoutes.begin();itr!=allRoutes.end();itr++){
			RoutingTableEntry advEntry= itr->second;
			if(advEntry.isChanged() && (!advRoutingTable.anyRunningEvent(advEntry.getDsptIp()))){
				if(!(advEntry.getSeqNumber()%2)){
					advEntry.setChangedState(false);
					routingTable.updateRoute(advEntry);
				}
				advRoutingTable.deleteRouteEntry(advEntry.getDsptIp());
			}
		}
	}
}

void RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4){
	NS_ASSERT(ipv4!=0);
	NS_ASSERT(ptrIp == 0);
	ptrIp = ipv4;
	NS_ASSERT(ptrIp->GetNInterfaces() == 1 && ptrIp->GetAddress(0,0).GetLocal() == Ipv4Address("127.0.0.1"));
	lo = ptrIp->GetNetDevice(0);
	NS_ASSERT(lo!=0);
	RoutingTableEntry rt(lo,Ipv4Address::GetLoopback(),0,Metric(0),Ipv4InterfaceAddress(Ipv4Address::GetLoopback(),Ipv4Mask("255.0.0.0")),Ipv4Address::GetLoopback(),Simulator::GetMaximumSimulationTime());
	rt.setChangedState(false);
	routingTable.addRouteEntry(rt);
	Simulator::ScheduleNow(&RoutingProtocol::Start,this);
}
void RoutingProtocol::Start(){

}

/*
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
}*/
