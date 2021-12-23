#include "CronSchedule.h"
//////////////////////////////////////////////////////////////////////////
CronJob::CronJob()
{
	m_pCronParser = NULL;
}

CronJob::~CronJob()
{
	Reset();
}

void CronJob::Reset()
{
	m_nUserData = nullptr;
	m_nJobId = 0;
	m_fnCallback = NULL;
	m_tmNext = 0;
	m_bDeleteFlag = false;

	if(NULL != m_pCronParser)
	{
		cron_expr_free(m_pCronParser);
	}
	m_pCronParser = NULL;
}

bool CronJob::Parse(const char* _pszCronExpr)
{
	Reset();

	const char* pszErr = NULL;
	m_pCronParser = cron_parse_expr(_pszCronExpr, &pszErr);
	if(NULL != pszErr)
	{
		return false;
	}

	return true;
}

bool CronJob::IsTriggerable()
{
	if(m_bDeleteFlag)
	{
		return false;
	}

	time_t tn;
	time(&tn);

	if(tn > m_tmNext)
	{
		return true;
	}
	return false;
}

int CronJob::Trigger()
{
	if(NULL == m_fnCallback)
	{
		return 0;
	}
	if(m_bDeleteFlag)
	{
		return 0;
	}

	return m_fnCallback(m_nJobId, m_nUserData);
}

void CronJob::GetNext()
{
	if(NULL == m_pCronParser)
	{
		return;
	}

	if(m_bDeleteFlag)
	{
		return;
	}
	
	//	only update next trigger time when is triggerable
	time_t tn;
	time(&tn);

	if(tn < m_tmNext)
	{
		return;
	}

	m_tmNext = cron_next(m_pCronParser, tn);
}


CronJobScheduler::CronJobScheduler()
{
	m_bJobWaitDelete = 0;
}

CronJobScheduler::~CronJobScheduler()
{
	Clear();
}

bool CronJobScheduler::AddCronJob(int _nJobId, const char* _pszCronExpr, FUNC_CRONCALLBACK _fnCb, void* _nArg)
{
	CronJob* pJob = new CronJob;
	if(!pJob->Parse(_pszCronExpr))
	{
		delete pJob;
		pJob = NULL;
		return false;
	}

	pJob->m_nJobId = _nJobId;
	pJob->m_nUserData = _nArg;
	pJob->m_fnCallback = _fnCb;
	pJob->GetNext();
	m_xCronJobList.push_back(pJob);

	//	sort all jobs by tm_next
	sortJobs();

	return true;
}

int CronJobScheduler::RemoveCronJob(int _nJobId)
{
	int nCount = 0;
	CronJobList::iterator it = m_xCronJobList.begin();

	for(it;
		it != m_xCronJobList.end();
		++it)
	{
		CronJob* pJob = *it;

		if(pJob->m_nJobId == _nJobId)
		{
			//	remove, this function may be invoked in callback function,
			//	so we can't remove it directly, we need delete it later
			pJob->m_bDeleteFlag = true;
			m_bJobWaitDelete = true;
			++nCount;
		}
	}

	return nCount;
}

void CronJobScheduler::Clear()
{
	//	directly clear all jobs, DONT call it from a job callback
	CronJobList::iterator it = m_xCronJobList.begin();

	for(it;
		it != m_xCronJobList.end();
		++it)
	{
		CronJob* pJob = *it;
		delete pJob;
	}

	m_xCronJobList.clear();
}

void CronJobScheduler::Update()
{
	CronJobList::iterator it = m_xCronJobList.begin();

	if(m_bJobWaitDelete)
	{
		//	delete jobs first
		for(it;
			it != m_xCronJobList.end();
			)
		{
			CronJob* pJob = *it;
			
			if(pJob->m_bDeleteFlag)
			{
				delete pJob;
				it = m_xCronJobList.erase(it);
			}
			else
			{
				++it;
			}
		}

		m_bJobWaitDelete = false;
	}

	//	test all jobs
	time_t tn;
	time(&tn);
	bool bTriggered = false;

	it = m_xCronJobList.begin();
	for(it;
		it != m_xCronJobList.end();
		++it)
	{
		CronJob* pJob = *it;

		if(pJob->IsTriggerable())
		{
			if(0 != pJob->Trigger())
			{
				pJob->m_bDeleteFlag = true;
				m_bJobWaitDelete = true;
			}
			else
			{
				pJob->GetNext();
			}
			
			bTriggered = true;
		}
		else
		{
			//	already sorted, so we can break the loop when job cannot be triggered
			break;
		}
	}

	//	if triggered, we need sort it again
	sortJobs();
}



void CronJobScheduler::sortJobs()
{
	m_xCronJobList.sort(CronJob::CronJobSorter());
}