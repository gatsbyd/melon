#include <algorithm>
#include <fcntl.h>
#include <gd.h>
#include <gdfonts.h>
#include <iostream>
#include <math.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Address.h"
#include "Log.h"
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
    ratioX_(static_cast<double>(samplingPeriod_ * (width_ - kLeftMargin_ - kRightMargin_)) / totalSeconds_)
{
  // gdImageSetAntiAliased(image_, black_);
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
using namespace std;

bool processExists(pid_t pid)
{
  	char filename[256];
  	snprintf(filename, sizeof filename, "/proc/%d/stat", pid);
  	return ::access(filename, R_OK) == 0;
}

class Procmon {
public:
	Procmon(Scheduler* scheduler, int pid, uint16_t port)
		: server_(IpAddress(port), scheduler),
		  clock_tick_per_seconds_(static_cast<int>(::sysconf(_SC_CLK_TCK))),
		  pid_(pid),
		  ticks_(0),
	      procname_(getProcname(readProcFile("stat"))) {

		}

	void start() {

	}

	ssize_t readFile(string filename, string& content) {
		int fd = ::open(filename.c_str(), O_RDONLY | O_CLOEXEC);
		char buf[65535];
		ssize_t readn = 0;
		ssize_t n = 0;
		while ( (n = ::read(fd, buf, sizeof(buf)) > 0 ) ) {
			content.append(buf, n);
			readn += n;
		}

		::close(fd);
		return readn;
	}

	string readProcFile(const char* basename)
	{
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
			name = stat.substr(lp, rp - lp);
		}
		return name;
	}

private:
	TcpServer server_;
	int clock_tick_per_seconds_;
	int pid_;
	int ticks_;
	const string procname_;
};

int main(int argc, char* argv[]) {
	//Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
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
	Procmon procmon(&scheduler, pid, port);
	procmon.start();
	scheduler.start();
	return 0;
}
