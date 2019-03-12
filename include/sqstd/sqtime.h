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
        获得本日的开始时间
    */
    inline time_t get_day_start_time(time_t val = time(NULL))
    {
        if(val <= 0) return 0;
        return (val - get_day_past_sec(val));
    }

    /*
        获得本周的开始时间
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
    获得本周的结束时间
    */
    inline time_t get_week_end_time(time_t val = time(NULL))
    {
        return get_week_start_time(val) + 3600 * 24 * 7;
    }

    /*
        获得本月的开始时间
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
        根据秒转化分钟，不满足1分钟的进位为1分钟
    */
    inline int64 sec_to_min(int64 seconds)
    {
        snqu::int64 minutes = seconds/60;
        if (0 != (seconds % 60) || 0 == seconds)
        {// 不足1分钟按1分钟计
            ++minutes;
        }
        return minutes;
    }

    /*
        根据时间取得当前周
    */
    inline int get_week(int64 time)
    {
        auto tm_val = get_tm(time);
        snqu::int32 day_of_week = tm_val.tm_wday;						// 周几
        return day_of_week;
    }

    /*
        根据时间取得当前周的小时索引
    */
    inline int get_hour_of_week(int64 time)
    {
        auto tm_val = get_tm(time);
        snqu::int32 day_of_week = tm_val.tm_wday;						// 周几
        snqu::int32 hour_of_week = day_of_week * 24 + tm_val.tm_hour;	// 一周的第几个小时，费率按此设置
        return hour_of_week;
    }

    /*
        根据整点时间取得当前周的小时索引
    */
    inline int get_hour_of_week_by_hour(int64 hour)
    {
        auto tm_val = get_tm(time(NULL));
        snqu::int32 day_of_week = tm_val.tm_wday;			// 周几
        snqu::int64 hour_of_week = day_of_week * 24 + hour;	// 一周的第几个小时，费率按此设置
        return (int)hour_of_week;
    }

    /*
        取公元元年（1年）到现在的周数
    */
    inline int get_week_index_since_zero(std::tm& tm_point)
    {
        // 润年数量
        auto cur_year = 1900 + tm_point.tm_year;
        auto ryears = cur_year/4 - cur_year/100 + cur_year/400;
        // 总天数
        auto total_days = cur_year* 365 + tm_point.tm_yday + ryears;
        auto total_week = total_days/7;
        if (total_days % 7 > 0)
            total_week++;
        return total_week;
    }

    inline double time_to_double(time_t end_time)
    {
        // 1970 到 double 起始时间 1899-12-30 的时差
        double st_1970 = 25569;
        auto time_zone = end_time + (8*3600);
        auto day_past = time_zone/86400;
        auto day_sec_past = time_zone%86400;
        double day_sec_past_db = day_sec_past/86400.0;
        return st_1970 + day_past + day_sec_past_db;
    }

    /*
        取得小时差，处理了跨0点的情况
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
    时间段a:开始a1 结束a2
    时间段b:开始b1 结束b2
    时间段a与时间段b的交集出现在 a2>b1 && a1<b2 的时候
    18-17的定额 3小时优先 当前时间是 16点30
    */
    inline bool has_time_intersection(int64 a1, int64 a2, int64 b1, int64 b2)
    {
        if (a2>b1 && a1<b2)
            return true;
        return false;
    }

    /* 兼容xp GetTickCount64 */
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