#include "RoutingTableEntry.h"

RoutingTableEntry::RoutingTableEntry(ns3::Ptr<ns3::NetDevice> dev,ns3::Ipv4Address destIp,uint32_t seqNumber,Metric met,ns3::Ipv4InterfaceAddress interface,ns3::Ipv4Address nextHop,ns3::Time lifetime,ns3::Time settlingTime,bool changed):
	srcIp(destIp),
	seqNumber(seqNumber),
	metric(met),
	outputLinkInterface(interface),
	lifeTime(lifetime),
	settlingTime(settlingTime),
	changed(changed)
{
	route = ns3::Create<ns3::Ipv4Route>();
	route->SetGateway(nextHop);
	route->SetDestination(srcIp);
	route->SetSource(outputLinkInterface.GetLocal());
	route->SetOutputDevice(dev);
	this->state = EntryState::VALID;
}
RoutingTableEntry::RoutingTableEntry(){
}	
inline uint32_t RoutingTableEntry:: getSeqNumber() const{
	return this->seqNumber;
}
inline ns3::Ptr<ns3::NetDevice> RoutingTableEntry::getDevice() const{
	return route->GetOutputDevice();
}
inline ns3::Ipv4InterfaceAddress RoutingTableEntry::getLink() const{
	return outputLinkInterface;
}
inline ns3::Ipv4Address RoutingTableEntry::getDsptIp() const{
	return this->srcIp;
}
inline ns3::Ptr<ns3::Ipv4Route> RoutingTableEntry::getRoute() const{
	return route;
}
inline ns3::Time RoutingTableEntry::getLifeTime() const{
	return ns3::Simulator::Now() - lifeTime;
}
inline ns3::Ipv4Address RoutingTableEntry::getNextHop() const{
	return route->GetGateway();
}
void RoutingTableEntry:: setSeqNumber(uint32_t seq_number){
	this->seqNumber = seq_number;
}
void RoutingTableEntry::setDevice(ns3::Ptr<ns3::NetDevice> dev){
	route->SetOutputDevice(dev);
}
void RoutingTableEntry::setLink(ns3::Ipv4InterfaceAddress link){
	outputLinkInterface = link;
}
inline void RoutingTableEntry::setLifeTime(ns3::Time time){
	lifeTime = time;
}
inline void RoutingTableEntry::setNextHop(ns3::Ipv4Address hop){
	route->SetGateway(hop);
}
inline void RoutingTableEntry::setSettlingTime(ns3::Time time){
	settlingTime = time;
}
inline void RoutingTableEntry::setChangedState(bool changed){
	this->changed = changed;
}
inline bool RoutingTableEntry::isChanged(){
	return changed;
}
inline ns3::Time RoutingTableEntry::getSettlingTime(){
	return settlingTime;
}
void RoutingTableEntry::setMetric(Metric& met)
{
     metric=met;
}
Metric RoutingTableEntry:: getMetric()
{
     return metric;
}
inline void RoutingTableEntry::setRoute(ns3::Ptr<ns3::Ipv4Route> route){
	this->route = route;
}
void RoutingTableEntry::print(ns3::Ptr<ns3::OutputStreamWrapper> stream) const{
	*stream->GetStream()<<std::setiosflags(std::ios::fixed)<<route->GetDestination()<<"\t\t"<<route->GetGateway()<<"\t\t"<<outputLinkInterface.GetLocal()<<"\t\t"<<std::setiosflags(std::ios::left)<<std::setw(10)<<metric.getMagnitude()<<"\t"<<std::setw(10)<<seqNumber<<"\t"<<std::setprecision(3)<<(ns3::Simulator::Now()-lifeTime).GetSeconds()<<"s\t\t"<<settlingTime.GetSeconds()<<"s\n";
}
