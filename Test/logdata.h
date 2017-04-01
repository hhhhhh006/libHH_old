#ifndef LOGDATA_H__
#define LOGDATA_H__


struct Logdata
{
	int id;
	std::string name;
	int time;
};


struct Loglive : public Logdata
{
	int flows;
	int num;
};

#endif // logdata_h__
