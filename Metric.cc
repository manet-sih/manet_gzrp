#include "Metric.h"
Metric::Metric(uint32_t hops){
	hopsCount = hops;
}
uint32_t Metric::getMagnitude() const{
	return hopsCount;
}
void Metric::setMagnitude(uint32_t val){
	hopsCount = val;
}
