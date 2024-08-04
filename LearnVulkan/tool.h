#ifdef DEBUG
	#define Log(x) std::cout<<x<<std::endl;
#else
	#define Log(x) 
#endif // DEBUG


#define RTE(x) throw std::runtime_error(x);

