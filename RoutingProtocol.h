#include <ns3/ipv4-routing-protocol.h>
#include <ns3/random-variable-stream.h>
#include <ns3/timer.h>
#include "RoutingTable.h"
class RoutingProtocol:public ns3::Ipv4RoutingProtocol{
	private:
		ns3::Ipv4Address nodeAddress;
		ns3::Ptr<ns3::Ipv4> ptrIp;
		ns3::Ptr<ns3::NetDevice> lo;
		RoutingTable routingTable;
		RoutingTable advRoutingTable;
		std::map<ns3::Ptr<ns3::Socket>,ns3::Ipv4InterfaceAddress> intrazoneSocketMap;
		std::map<ns3::Ptr<ns3::Socket>,ns3::Ipv4InterfaceAddress> interzoneSocketMap;
		std::map<ns3::Ipv4Address,uint32_t> serviceMap;
		ns3::Time settlingTime;
		ns3::Timer updateTimer;
		ns3::Time periodicUpdateInterval;
		ns3::Ptr<ns3::UniformRandomVariable> random_variable;
		uint32_t myZoneId;
	private:
		void Start(); //not implemented
		ns3::Ptr<ns3::Socket> findInterzoneSocket(ns3::Ipv4InterfaceAddress interface) const;	//implemented
		ns3::Ptr<ns3::Socket> findIntrazoneSocket(ns3::Ipv4InterfaceAddress interface) const;	//implemented
		void SendTriggeredUpdate();
		void recvUpdates(ns3::Ptr<ns3::Socket> socket); //not implemented
		void recvInterzoneControlPackets(ns3::Ptr<ns3::Socket> socket); //not implemented
		uint32_t getZoneId() const; //not implemented
		void setZoneId();  //not implemented
		void getLocation(float& x,float& y) const; //not implemented
		uint32_t calculateZoneId(const float x,const float y) const; //not implemented
		void drop(ns3::Ptr<const ns3::Packet>, const ns3::Ipv4Header &, ns3::Socket::SocketErrno); //not implemented
		void send(ns3::Ptr<ns3::Ipv4Route>, ns3::Ptr<const ns3::Packet>, const ns3::Ipv4Header &); //not implemented
		void sendPeriodicUpdates(); //not implemented
		void mergeTriggerPeriodicUpdates();
	public:	
		RoutingProtocol();//not implemented
		~RoutingProtocol();//not implemented
   		static ns3::TypeId GetTypeId (void);//extra
  		static const uint32_t INTRAZONE_PORT;
  		static const uint32_t INTERZONE_PORT;
		virtual void NotifyAddAddress(uint32_t interface,ns3::Ipv4InterfaceAddress); //implemented
		virtual void NotifyRemoveAddress(uint32_t interfaceNo, ns3::Ipv4InterfaceAddress);//implemented
		virtual void NotifyInterfaceUp(uint32_t );//implemented
		virtual void NotifyInterfaceDown(uint32_t );//implemented
		virtual void SetIpv4(ns3::Ptr<ns3::Ipv4>);//not implemented
  		bool RouteInput (ns3::Ptr<const ns3::Packet> p, const ns3::Ipv4Header &header, ns3::Ptr<const ns3::NetDevice> idev, UnicastForwardCallback ucb,MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb); //not implemented
		ns3::Ptr<ns3::Ipv4Route> LoopbackRoute (const ns3::Ipv4Header & header, ns3::Ptr<ns3::NetDevice> oif) const; //not implemented
		ns3::Ptr<ns3::Ipv4Route> RouteOutput (ns3::Ptr<ns3::Packet> p, const ns3::Ipv4Header &header, ns3::Ptr<ns3::NetDevice> oif,ns3::Socket::SocketErrno &sockerr); //not implemented
		void DoDispose(); //not implemented
};
