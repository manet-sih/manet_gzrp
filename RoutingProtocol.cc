#include <set>
#include <ns3/object.h>
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
		RoutingTableEntry fwdEntry,advEntry;
		bool entryFound = routingTable.search(header.getSrcIp(),fwdEntry);
		if(entryFound == false){
			if(header.getSeqNo() % 2 != 1){
				if(header.getZoneId() != getZoneId() && header.getSrcIp()==sender){
					std::set<uint32_t> ipSet;
					routingTable.getZonesForIp(ipSet,header.getSrcIp());
					if(ipSet.size()==0){
						routingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
						advRoutingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
						Simulator::Schedule(settlingTime,&RoutingProtocol::SendTriggeredUpdate,this);
					}else if(ipSet.size()!=1 || (*(ipSet.begin()) != header.getZoneId())){
						routingTable.deleteIpFromZoneMap(header.getSrcIp());
						advRoutingTable.deleteIpFromZoneMap(header.getSrcIp());
						routingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
						advRoutingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
						Simulator::Schedule(settlingTime,&RoutingProtocol::SendTriggeredUpdate,this);
					}
					continue;
				}else if(header.getZoneId()==getZoneId()){
					RoutingTableEntry re (dev,header.getSrcIp(),header.getSeqNo(),header.getMetric(),ptrIp->GetAddress(ptrIp->GetInterfaceForAddress(receiver),0),sender,ns3::Simulator::Now(),settlingTime,true);
					if(sender==header.getSrcIp()){
						routingTable.deleteIpFromZoneMap(header.getSrcIp());
						advRoutingTable.deleteIpFromZoneMap(header.getSrcIp());
						std::set<uint32_t> neighbourZones;
						header.getNeighbourZones(neighbourZones);
						for(uint32_t i : neighbourZones){
							routingTable.addZoneIp(header.getSrcIp(),i);
							advRoutingTable.addZoneIp(header.getSrcIp(),i);
						}
					}
					routingTable.addRouteEntry(re);
					advRoutingTable.addRouteEntry(re);
				}
			}else{
				routingTable.deleteIpFromZoneMap(header.getSrcIp());
			}
		}else{
			//Adding route in advRoutingTable if not present in advRoutinTable but present in routingTable
			if(!advRoutingTable.search(header.getSrcIp(),advEntry)){
				advRoutingTable.addRouteEntry(fwdEntry);
				advRoutingTable.search(header.getSrcIp(),advEntry);
			}
			if(header.getSeqNo() %2 !=1){
				if(header.getSeqNo() > advEntry.getSeqNumber()){
					if(header.getZoneId() != getZoneId() && header.getSrcIp()==sender){
						routingTable.deleteIpFromZoneMap(header.getSrcIp());
						advRoutingTable.deleteIpFromZoneMap(header.getSrcIp());
						routingTable.deleteRouteEntry(header.getSrcIp());
						advEntry.setEntryState(EntryState::INVALID);
						advRoutingTable.updateRoute(advEntry);
						event = Simulator::Schedule(settlingTime,&RoutingProtocol::SendTriggeredUpdate,this);
						advRoutingTable.addEvent(header.getSrcIp(),event);
						routingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
						advRoutingTable.addZoneIp(header.getSrcIp(),header.getZoneId());
						continue;
					}else if(header.getZoneId() == getZoneId()){
						if(sender==header.getSrcIp()){
							std::set<uint32_t> zoneSet;
							std::set<uint32_t> presentZoneSet;
							header.getNeighbourZones(zoneSet);
							routingTable.getZonesForIp(presentZoneSet,header.getSrcIp());
							bool sendLocationUpdate = false;
							if(zoneSet!=presentZoneSet){
								for(uint32_t zone : zoneSet){
									auto findItr = presentZoneSet.find(zone);
									if(findItr == presentZoneSet.end()){
										if(!sendLocationUpdate){
											std::set<Ipv4Address> ipSet;
											bool foundIp = routingTable.getAllIpforZone(zone,ipSet);
											if(!foundIp) sendLocationUpdate = true;
										}
										routingTable.addZoneIp(header.getSrcIp(),zone);
										advRoutingTable.addZoneIp(header.getSrcIp(),zone);
									}
								}
								for(uint32_t zone : presentZoneSet){
									auto findItr = zoneSet.find(zone);
									if(findItr == zoneSet.end()){
										routingTable.deleteZoneIp(header.getSrcIp(),zone);
										advRoutingTable.deleteZoneIp(header.getSrcIp(),zone);
										if(!sendLocationUpdate){
											std::set<Ipv4Address> ipSet;
											bool foundIp = routingTable.getAllIpforZone(zone,ipSet);
											if(!foundIp) sendLocationUpdate = true;
										}
									}
								}
							}
							if(sendLocationUpdate){
								Simulator::Schedule(settlingTime,&RoutingProtocol::sendTriggeredLocationUpdate,this);
							}
						}
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
					}
				}else if(header.getSeqNo() == fwdEntry.getSeqNumber()){
					if(header.getZoneId()==getZoneId()){
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
					}
				}else{
					if(header.getZoneId()==getZoneId()){
						if(!advRoutingTable.anyRunningEvent(header.getSrcIp())){
							advRoutingTable.deleteEvent(header.getSrcIp());
						}
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
			std::set<uint32_t> zoneSet;
			if(routingTable.getZoneList(zoneSet)){
				for(uint32_t zone : zoneSet){
					header.addZone(zone);
				}
			}
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
				std::set<uint32_t> zoneSet;
				if(routingTable.getZoneList(zoneSet)){
					for(uint32_t zone : zoneSet){
						header.addZone(zone);
					}
				}
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
	unicastCallback = MakeCallback(&RoutingProtocol::send,this);
	errorCallback = MakeCallback(&RoutingProtocol::drop,this);
	updateTimer.SetFunction(&RoutingProtocol::sendPeriodicUpdates,this);
	updateTimer.Schedule(MicroSeconds(random_variable->GetInteger(0,1000)));
	routingTable.setHoldTime(Time(holdTime*periodicUpdateInterval));
	advRoutingTable.setHoldTime(Time(holdTime*periodicUpdateInterval));
}
void RoutingProtocol::DoDispose(){
	ptrIp = 0;
	for(auto itr = intrazoneSocketMap.begin();itr!=intrazoneSocketMap.end();itr++){
		itr->first->Close();
	}
	intrazoneSocketMap.clear();
	for(auto itr = interzoneSocketMap.begin();itr!=interzoneSocketMap.end();itr++){
		itr->first->Close();
	}
	interzoneSocketMap.clear();
	Ipv4RoutingProtocol::DoDispose();
}
RoutingProtocol::RoutingProtocol():
	routingTable(),
	advRoutingTable(),
	updateTimer(Timer::CHECK_ON_DESTROY){
		random_variable = CreateObject<UniformRandomVariable>();
	}

void RoutingProtocol::PrintRoutingTable(ns3::Ptr<ns3::OutputStreamWrapper> stream,ns3::Time::Unit unit) const{
	*stream->GetStream()<<"Node: "<<ptrIp->GetObject<Node>()->GetId()
		<<", Time: "<<Now().As(unit)
		<<", Local time: "<<GetObject<Node>()->GetLocalTime().As(unit)
		<<", Routing table"<<std::endl;
	routingTable.print(stream);
	*stream->GetStream()<<std::endl;
}
int64_t RoutingProtocol::assignStreams(int64_t stream){
	random_variable->SetStream(stream);
	return 1;
}
void RoutingProtocol::sendTriggeredLocationUpdate(){
	for(auto j = intrazoneSocketMap.begin();j!=intrazoneSocketMap.end();j++){
		GzrpPacket header;
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		Ptr<Packet> packet = Create<Packet>();
		RoutingTableEntry temp2;
		routingTable.search(ptrIp->GetAddress(1,0).GetBroadcast(),temp2);
		header.setSrcIp(ptrIp->GetAddress(1,0).GetLocal());
		header.setSeqNo(temp2.getSeqNumber()+2);
		header.setMetric((temp2.getMetric()+1).getMagnitude());
		std::set<uint32_t> zoneSet;
		if(routingTable.getZoneList(zoneSet)){
			for(uint32_t zone : zoneSet){
				header.addZone(zone);
			}
		}
		packet->AddHeader(header);
		Ipv4Address destination;
		if(iface.GetMask() == Ipv4Mask::GetOnes()){
			destination = Ipv4Address("255.255.255.255");
		}else{
			destination = iface.GetBroadcast();
		}
		socket->SendTo(packet,0,InetSocketAddress(destination,INTRAZONE_PORT));
		temp2.setSeqNumber(header.getSeqNo());
		routingTable.updateRoute(temp2);
	}
}
