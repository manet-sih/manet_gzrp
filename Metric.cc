#include "./Metric.h"
Metric::Metric(){}
Metric::Metric(uint32_t hops):hopsCount(hops){}
void Metric::setMetric(uint32_t hops){
	hopsCount=hops;
}	
Metric Metric::getMetric(){
	return Metric(hopsCount);
}
