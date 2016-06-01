# CronScheduler
Using cron expression to schedule functions. c++

## imported source

This code using a cron parser, author staticlibs.net 

[ccronexpr](github.com/staticlibs/ccronexpr)

## purpose

I want a lib to schedule my own function with cron expression, but i can't find anyone written by c c++,almost every one is a scheduler to schedule executable files.

I found a ccronexpr lib and it looks very awesome.So i simply wrap it using c++.

## example

	#include "CronSchedule.h"
	
	CronJobScheduler sche;
	
	int cron_cb1(int _id, int _arg)
	{
		time_t tn;
		time(&tn);
	
		static int s_nCount = 0;
		++s_nCount;
	
		printf("job %d active with arg %d, count %d, time %d\n", _id, _arg, s_nCount, tn);
	
		if(5 == s_nCount)
		{
			printf("delete job %d\n", _id);
			sche.RemoveCronJob(_id);
		}
	
		return 0;
	}
	
	int cron_cb2(int _id, int _arg)
	{
		time_t tn;
		time(&tn);
	
		static int s_nCount = 0;
		++s_nCount;
	
		printf("job %d active with arg %d, count %d, time %d\n", _id, _arg, s_nCount, tn);
	
		if(5 == s_nCount)
		{
			printf("delete job %d\n", _id);
			return 1;
		}
	
		return 0;
	}
	
	int cron_cb3(int _id, int _arg)
	{
		time_t tn;
		time(&tn);
	
		static int s_nCount = 0;
		++s_nCount;
	
		printf("job %d active with arg %d, count %d, time %d\n", _id, _arg, s_nCount, tn);
	
		if(5 == s_nCount)
		{
			printf("delete job %d\n", _id);
			return 1;
		}
	
		return 0;
	}
	
	int main()
	{
		sche.AddCronJob(1, "*/5 * * * * *", cron_cb1, 2);
		sche.AddCronJob(2, "*/10 * * * * *", cron_cb2, 2);
		sche.AddCronJob(3, "*/2 * * * * *", cron_cb3, 2);
	
		while(1)
		{
			sche.Update();
			Sleep(1000);
		}
	}