#include <iostream>
class Metric{
	private:uint32_t hopsCount;
	public:
		Metric();
		Metric(uint32_t hops);
		void setMetric(Metric&);
		Metric getMetric();
		bool operator <(const Metric& met){
			return (met.hopsCount>this->hopsCount)?true:false;
		}
};