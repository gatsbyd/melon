#include <algorithm>
#include <assert.h>
#include <fcntl.h>
#include <functional>
#include <gd.h>
#include <gdfonts.h>
#include <iostream>
#include <math.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Address.h"
#include "Log.h"
#include "http/HttpConnection.h"
#include "Singleton.h"
#include "Scheduler.h"
#include "TcpServer.h"

typedef struct gdImageStruct* gdImagePtr;

class Plot {
public:
	Plot(int width, int height, int totalSeconds, int samplingPeriod);
	~Plot();
	std::string plotCpu(const std::vector<double>& data);

private:
	std::string toPng();
	int getX(ssize_t x, ssize_t total) const;
	int getY(double value) const;
	void label(double maxValue);
	struct MyGdFont;
  	typedef struct MyGdFont* MyGdFontPtr;

  	const int width_;
  	const int height_;
  	const int totalSeconds_;
  	const int samplingPeriod_;
  	gdImagePtr const image_;
  	MyGdFontPtr const font_;
  	const int fontWidth_;
  	const int fontHeight_;
  	const int background_;
  	const int black_;
  	const int gray_;
  	const int blue_;

  	const int kRightMargin_;
  	static const int kLeftMargin_ = 5;
  	static const int kMarginY_ = 5;

  	const double ratioX_;
};

struct Plot::MyGdFont : public gdFont {};

Plot::Plot(int width, int height, int totalSeconds, int samplingPeriod)
  : width_(width),
    height_(height),
    totalSeconds_(totalSeconds),
    samplingPeriod_(samplingPeriod),
    image_(gdImageCreate(width_, height_)),
    font_(static_cast<MyGdFont*>(gdFontGetSmall())),
    fontWidth_(font_->w),
    fontHeight_(font_->h),
    background_(gdImageColorAllocate(image_, 255, 255, 240)),
    black_(gdImageColorAllocate(image_, 0, 0, 0)),
    gray_(gdImageColorAllocate(image_, 200, 200, 200)),
    blue_(gdImageColorAllocate(image_, 128, 128, 255)),
    kRightMargin_(3 * fontWidth_ + 5),
    ratioX_(static_cast<double>(samplingPeriod_ * (width_ - kLeftMargin_ - kRightMargin_)) / totalSeconds_){
	}

Plot::~Plot()
{
	gdImageDestroy(image_);
}

std::string Plot::plotCpu(const std::vector<double>& data)
{
  gdImageFilledRectangle(image_, 0, 0, width_, height_, background_);
  if (data.size() > 1)
  {
    gdImageSetThickness(image_, 2);
    double max = *std::max_element(data.begin(), data.end());
    if (max >= 10.0)
    {
      max = ceil(max);
    }
    else
    {
      max = std::max(0.1, ceil(max*10.0) / 10.0);
    }
    label(max);

    for (size_t i = 0; i < data.size()-1; ++i)
    {
      gdImageLine(image_,
                  getX(i, data.size()),
                  getY(data[i] / max),
                  getX(i+1, data.size()),
                  getY(data[i+1]/max),
                  black_);
    }
  }

  int total = totalSeconds_/samplingPeriod_;
  gdImageSetThickness(image_, 1);
  gdImageLine(image_, getX(0, total), getY(0)+2, getX(total, total), getY(0)+2, gray_);
  gdImageLine(image_, getX(total, total), getY(0)+2, getX(total, total), getY(1)+2, gray_);
  return toPng();
}

void Plot::label(double maxValue)
{
    char buf[64];
    if (maxValue >= 10.0)
      snprintf(buf, sizeof buf, "%.0f", maxValue);
    else
      snprintf(buf, sizeof buf, "%.1f", maxValue);

    gdImageString(image_,
                  font_,
                  width_ - kRightMargin_ + 3,
                  kMarginY_ - 3,
                  reinterpret_cast<unsigned char*>(buf),
                  black_);

    snprintf(buf, sizeof buf, "0");
    gdImageString(image_,
                  font_,
                  width_ - kRightMargin_ + 3,
                  height_ - kMarginY_ - 3 - fontHeight_ / 2,
                  reinterpret_cast<unsigned char*>(buf),
                  gray_);

    snprintf(buf, sizeof buf, "-%ds", totalSeconds_);
    gdImageString(image_,
                  font_,
                  kLeftMargin_,
                  height_ - kMarginY_ - fontHeight_,
                  reinterpret_cast<unsigned char*>(buf),
                  blue_);
}

int Plot::getX(ssize_t i, ssize_t total) const
{
  double x = (width_ - kLeftMargin_ - kRightMargin_) + static_cast<double>(i - total) * ratioX_;
  return static_cast<int>(x + 0.5) + kLeftMargin_;
}

int Plot::getY(double value) const
{
  return static_cast<int>((1.0 - value) * (height_-2*kMarginY_) + 0.5) + kMarginY_;
}

std::string Plot::toPng()
{
  int size = 0;
  void* png = ::gdImagePngPtr(image_, &size);
  std::string result(static_cast<char*>(png), size);
  gdFree(png);
  return result;
}

using namespace melon;
using namespace melon::http;
using namespace std;

bool processExists(pid_t pid)
{
  	char filename[256];
  	snprintf(filename, sizeof filename, "/proc/%d/stat", pid);
  	return ::access(filename, R_OK) == 0;
}

struct StatData
{
  void parse(string content, int kbPerPage)
  {
	size_t rp = content.rfind(')');
    std::istringstream iss(content.data() + rp + 1);

    //            0    1    2    3     4    5       6   7 8 9  11  13   15
    // 3770 (cat) R 3718 3770 3718 34818 3770 4202496 214 0 0 0 0 0 0 0 20
    // 16  18     19      20 21                   22      23      24              25
    //  0 1 0 298215 5750784 81 18446744073709551615 4194304 4242836 140736345340592
    //              26
    // 140736066274232 140575670169216 0 0 0 0 0 0 0 17 0 0 0 0 0 0

    iss >> state;
    iss >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags;
    iss >> minflt >> cminflt >> majflt >> cmajflt;
    iss >> utime >> stime >> cutime >> cstime;
    iss >> priority >> nice >> num_threads >> itrealvalue >> starttime;
    long vsize, rss;
    iss >> vsize >> rss >> rsslim;
    vsizeKb = vsize / 1024;
    rssKb = rss * kbPerPage;
  }
  char state;
  int ppid;
  int pgrp;
  int session;
  int tty_nr;
  int tpgid;
  int flags;

  long minflt;
  long cminflt;
  long majflt;
  long cmajflt;

  long utime;
  long stime;
  long cutime;
  long cstime;

  long priority;
  long nice;
  long num_threads;
  long itrealvalue;
  long starttime;

  long vsizeKb;
  long rssKb;
  long rsslim;
};


class Procmon {
public:
	Procmon(Scheduler* scheduler, int pid, uint16_t port)
		: scheduler_(scheduler),
	   	  server_(IpAddress(port), scheduler),
		  kb_per_page_(static_cast<int>(::sysconf(_SC_PAGE_SIZE)) / 1024),
		  clock_tick_per_seconds_(static_cast<int>(::sysconf(_SC_CLK_TCK))),
		  pid_(pid),
		  ticks_(0),
	      procname_(getProcname(readProcFile("stat"))),
	      cpu_chart_(640, 100, 600, kPeriod_),
		  cpu_usage_max_size_(600 / kPeriod_) {
			memset(&last_stat_data_, 0, sizeof last_stat_data_);		
			server_.setConnectionHandler(std::bind(&Procmon::connectionHandler, this, std::placeholders::_1));
		}

	void start() {
		tick();
		scheduler_->runEvery(kPeriod_ * Timestamp::kMicrosecondsPerSecond, std::make_shared<Coroutine>(std::bind(&Procmon::tick, this)));
		server_.start();
	}

private:
	void connectionHandler(TcpConnection::Ptr conn) {
		HttpConnection::Ptr http_conn = std::make_shared<HttpConnection>(conn);	
		HttpRequest::Ptr request = http_conn->recvRequest();		

		HttpResponse::Ptr rsp = std::make_shared<HttpResponse>();
		rsp->setHttpStatus(HttpStatus::OK);
		if (request->getPath() == "/") {
			rsp->setHeader("Content-Type", "text/html");		
			string response;
			appendResponse(response, "<html><head><title>%s</title>\n", procname_.c_str());
			appendResponse(response, "<meta http-equiv=\"refresh\" content=\"%d\">\n", 3);
			response.append("</head><body>\n");
			response.append("<p><table>");

			appendTableRow(response, "Procname", procname_);
			appendTableRow(response, "PID", pid_);
			appendTableRow(response, "State", getState(last_stat_data_.state));
			appendTableRow(response, "User time (s)", last_stat_data_.utime / clock_tick_per_seconds_);
			appendTableRow(response, "System time (s)", last_stat_data_.stime / clock_tick_per_seconds_);
			appendTableRow(response, "VmSize (KiB)", last_stat_data_.vsizeKb);
			appendTableRow(response, "VmRSS (KiB)", last_stat_data_.rssKb);
			appendTableRow(response, "Threads", last_stat_data_.num_threads);
			appendTableRow(response, "Priority", last_stat_data_.priority);
			appendTableRow(response, "CPU usage", "<img src=\"/cpu.png\" height=\"100\" witdh=\"640\">");
			
			response.append("</table>");
			response.append("</body></html>");

			rsp->setContent(response);
		} else if (request->getPath() == "/cpu.png") {
			std::vector<double> cpu_usage;
			for (const auto& item : cpu_usage_)
				cpu_usage.push_back(item.cpuUsage(kPeriod_, clock_tick_per_seconds_));
			string png = cpu_chart_.plotCpu(cpu_usage);
			rsp->setHeader("Content-Type", "image/png");

			rsp->setContent(png);
		} else {
			rsp->setHttpStatus(HttpStatus::NOT_FOUND);
		}

		http_conn->sendResponse(rsp);

		conn->shutdown();
		Buffer::Ptr buffer = std::make_shared<Buffer>();
		while (conn->read(buffer) > 0) {

		}
		conn->close();
	}

	void tick() {
		string stat = readProcFile("stat");  
		if (stat.empty())
      		return;
    	StatData statData;
    	memset(&statData, 0, sizeof statData);
    	statData.parse(stat, kb_per_page_);
		if (ticks_ > 0)
    	{
      		CpuTime time;
      		time.userTime_ = std::max(0, static_cast<int>(statData.utime - last_stat_data_.utime));
      		time.sysTime_ = std::max(0, static_cast<int>(statData.stime - last_stat_data_.stime));
			if (cpu_usage_.size() >= cpu_usage_max_size_) {
				cpu_usage_.pop_front();
			}
			cpu_usage_.push_back(time);
			assert(cpu_usage_.size() <= cpu_usage_max_size_);
    	}

    	last_stat_data_ = statData;
    	++ticks_;	
	}

	void appendResponse(string& response, const char* fmt, ...) {
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		int n = vsnprintf(buf, sizeof buf, fmt, args);
		va_end(args);
		response.append(buf, n);
	}

	void appendTableRow(string& response, const char* name, long value) {
		appendResponse(response, "<tr><td>%s</td><td>%ld</td></tr>\n", name, value);
	}

	void appendTableRow(string& response, const char* name, string value) {
		appendResponse(response, "<tr><td>%s</td><td>%s</td></tr>\n", name, value.c_str());
  	}

	ssize_t readFile(string filename, string& content) {
		FILE* fp = ::fopen(filename.c_str(), "r");
		char buf[65535];
		size_t n = ::fread(static_cast<void*>(buf), 1, sizeof buf, fp);
		content.append(buf, n);

		::fclose(fp);
		return n;
	}

	string readProcFile(const char* basename) {
		char filename[256];
		snprintf(filename, sizeof filename, "/proc/%d/%s", pid_, basename);
		string content;
		readFile(filename, content);
		return content;
	}

	string getProcname(const string& stat) {
		string name;
		size_t lp = stat.find('(');
		size_t rp = stat.rfind(')');
		if (lp != string::npos && rp != string::npos && lp < rp) {
			name = stat.substr(lp + 1, rp - lp - 1);
		}
		return name;
	}

	static const char* getState(char state) {
		switch (state)
		{
			case 'R':
				return "Running";
			case 'S':
				return "Sleeping";
			case 'D':
				return "Disk sleep";
			case 'Z':
				return "Zombie";
			default:
				return "Unknown";
		}
	}

	struct CpuTime {
		int userTime_;
		int sysTime_;
		double cpuUsage(double kPeriod, double kClockTicksPerSecond) const {
			return (userTime_ + sysTime_) / (kClockTicksPerSecond * kPeriod);
		}
	};

	Scheduler* scheduler_;
	TcpServer server_;
	const int kb_per_page_;
	const static int kPeriod_ = 2.0;
	const int clock_tick_per_seconds_;
	const int pid_;
	int ticks_;
	const string procname_;
	StatData last_stat_data_;
	Plot cpu_chart_;
	uint32_t cpu_usage_max_size_;
	list<CpuTime> cpu_usage_;
};

static void sighandler(int) {
	exit(0);
}

int main(int argc, char* argv[]) {
	signal(SIGUSR1, sighandler);
	//Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	if (argc < 3) {
		printf("Usage: %s pid port\n", argv[0]);
		return 0;
	}
	int pid = atoi(argv[1]);
	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	if (!processExists(pid)) {
		printf("process %d doesn't exist\n", pid);
		return 1;
	}

	Scheduler scheduler;
	scheduler.startAsync();
	Procmon procmon(&scheduler, pid, port);
	procmon.start();

	getchar();
	return 0;
}
