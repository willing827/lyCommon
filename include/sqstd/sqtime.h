#pragma once
#include <time.h>
#include <sqstd/sqtypes.h>
#include <chrono>

namespace snqu{

    inline std::tm get_tm(time_t param = time(NULL))
    {
        std::tm tm_val = {0};
        ::localtime_s(&tm_val, &param);
        return tm_val;
    }

    inline time_t get_day_past_sec(time_t val = time(NULL))
    {
        if (val <= 0)
            return 0;

        std::tm tm_val = {0};
        ::localtime_s(&tm_val, &val);

        return (tm_val.tm_sec + (tm_val.tm_hour * 3600) + (tm_val.tm_min * 60));
    }

    /*
        ��ñ��յĿ�ʼʱ��
    */
    inline time_t get_day_start_time(time_t val = time(NULL))
    {
        if(val <= 0) return 0;
        return (val - get_day_past_sec(val));
    }

    /*
        ��ñ��ܵĿ�ʼʱ��
    */
    inline time_t get_week_start_time(time_t val = time(NULL))
    {
        if (val <= 0)
            return 0;

        std::tm tm_val = {0};
        ::localtime_s(&tm_val, &val);

        int wday = tm_val.tm_wday;

        __int64 week_past_sec = wday * 3600 * 24 + get_day_past_sec(val);

        return val - week_past_sec;
    }

    /*
    ��ñ��ܵĽ���ʱ��
    */
    inline time_t get_week_end_time(time_t val = time(NULL))
    {
        return get_week_start_time(val) + 3600 * 24 * 7;
    }

    /*
        ��ñ��µĿ�ʼʱ��
    */
    inline time_t get_mon_start_time(time_t val = time(NULL))
    {
        if (val <= 0)
            return 0;

        std::tm tm_val = {0};
        ::localtime_s(&tm_val, &val);
        
        int64 mon_past_sec = (tm_val.tm_mday-1) * 3600 * 24 + get_day_past_sec(val);
        return val - mon_past_sec;
    }

    /*
        ������ת�����ӣ�������1���ӵĽ�λΪ1����
    */
    inline int64 sec_to_min(int64 seconds)
    {
        snqu::int64 minutes = seconds/60;
        if (0 != (seconds % 60) || 0 == seconds)
        {// ����1���Ӱ�1���Ӽ�
            ++minutes;
        }
        return minutes;
    }

    /*
        ����ʱ��ȡ�õ�ǰ��
    */
    inline int get_week(int64 time)
    {
        auto tm_val = get_tm(time);
        snqu::int32 day_of_week = tm_val.tm_wday;						// �ܼ�
        return day_of_week;
    }

    /*
        ����ʱ��ȡ�õ�ǰ�ܵ�Сʱ����
    */
    inline int get_hour_of_week(int64 time)
    {
        auto tm_val = get_tm(time);
        snqu::int32 day_of_week = tm_val.tm_wday;						// �ܼ�
        snqu::int32 hour_of_week = day_of_week * 24 + tm_val.tm_hour;	// һ�ܵĵڼ���Сʱ�����ʰ�������
        return hour_of_week;
    }

    /*
        ��������ʱ��ȡ�õ�ǰ�ܵ�Сʱ����
    */
    inline int get_hour_of_week_by_hour(int64 hour)
    {
        auto tm_val = get_tm(time(NULL));
        snqu::int32 day_of_week = tm_val.tm_wday;			// �ܼ�
        snqu::int64 hour_of_week = day_of_week * 24 + hour;	// һ�ܵĵڼ���Сʱ�����ʰ�������
        return (int)hour_of_week;
    }

    /*
        ȡ��ԪԪ�꣨1�꣩�����ڵ�����
    */
    inline int get_week_index_since_zero(std::tm& tm_point)
    {
        // ��������
        auto cur_year = 1900 + tm_point.tm_year;
        auto ryears = cur_year/4 - cur_year/100 + cur_year/400;
        // ������
        auto total_days = cur_year* 365 + tm_point.tm_yday + ryears;
        auto total_week = total_days/7;
        if (total_days % 7 > 0)
            total_week++;
        return total_week;
    }

    inline double time_to_double(time_t end_time)
    {
        // 1970 �� double ��ʼʱ�� 1899-12-30 ��ʱ��
        double st_1970 = 25569;
        auto time_zone = end_time + (8*3600);
        auto day_past = time_zone/86400;
        auto day_sec_past = time_zone%86400;
        double day_sec_past_db = day_sec_past/86400.0;
        return st_1970 + day_past + day_sec_past_db;
    }

    /*
        ȡ��Сʱ������˿�0������
    */
    inline int get_hour_merge(int64 begin, int64 end)
    {
        int64  diff_hour = 0;
        if (end < begin)
            diff_hour = end + 24 - begin;
        else
            diff_hour = end - begin;
        return (int)diff_hour;
    }

    /*
    ʱ���a:��ʼa1 ����a2
    ʱ���b:��ʼb1 ����b2
    ʱ���a��ʱ���b�Ľ��������� a2>b1 && a1<b2 ��ʱ��
    18-17�Ķ��� 3Сʱ���� ��ǰʱ���� 16��30
    */
    inline bool has_time_intersection(int64 a1, int64 a2, int64 b1, int64 b2)
    {
        if (a2>b1 && a1<b2)
            return true;
        return false;
    }

    /* ����xp GetTickCount64 */
    inline int64 GetTickCount64()
    {
        //_GetSysTickCount64()  
        LARGE_INTEGER TicksPerSecond = { 0 };
        LARGE_INTEGER Tick;
        if (!TicksPerSecond.QuadPart)
            QueryPerformanceFrequency(&TicksPerSecond);
        QueryPerformanceCounter(&Tick);
        __int64 Seconds = Tick.QuadPart / TicksPerSecond.QuadPart;
        __int64 LeftPart = Tick.QuadPart - (TicksPerSecond.QuadPart*Seconds);
        __int64 MillSeconds = LeftPart * 1000 / TicksPerSecond.QuadPart;
        __int64 Ret = Seconds * 1000 + MillSeconds;
        return Ret;
    }

    inline int64 GetSteadyNow()
    {
        using namespace std::chrono;
        auto now = steady_clock::now();
        auto zero = steady_clock::time_point();
        auto duration = duration_cast<seconds>(now - zero);
        return duration.count();
    }

	inline uint64_t GetTickCountMicroSeconds()
	{
		using namespace std::chrono;
		auto now = steady_clock::now();
		auto zero = steady_clock::time_point();
		auto duration = duration_cast<microseconds>(now - zero);
		return duration.count();
	}

	inline uint64_t GetTimeMilliSeconds()
	{
		auto now = time(NULL);
		SYSTEMTIME currentTime;
		GetSystemTime(&currentTime);
		auto relTime = now * 1000 + currentTime.wMilliseconds;
		return relTime;
	}
}