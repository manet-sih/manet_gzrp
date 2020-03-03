#include <iostream>
class Metric{
	private:uint32_t hopsCount;
	public:
		Metric(uint32_t hops = 0);
		void setMagnitude(uint32_t value);
		uint32_t getMagnitude() const;
		bool operator <(const Metric& met){
			return hopsCount<met.hopsCount;
		}
		bool operator>(const Metric& met){
			return hopsCount>met.hopsCount;
		}
		bool operator==(const Metric& met){
			return hopsCount==met.hopsCount;
		}
		Metric operator+(int32_t num){
			return Metric(hopsCount+num);
		}
};
