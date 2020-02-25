#include "./Metric.h"
Metric::Metric(){}
Metric::Metric(uint32_t hops):hopsCount(hops){}
void Metric::setMetric(Metric &m){
	hopsCount=m.hopsCount;
}	
Metric Metric::getMetric(){
	return Metric(hopsCount);
}
