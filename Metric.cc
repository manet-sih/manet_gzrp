#include "./Metric.h"
Metric::Metric(){}
Metric::Metric(uint32_t hops){
	hopsCount = hops;
}
void Metric::setMetric(Metric &m){
	hopsCount=hopsCount;
}	
Metric Metric::getMetric()const{
	return Metric(hopsCount);
}
