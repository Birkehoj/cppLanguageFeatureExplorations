#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <fstream>
#include <chrono>

namespace UserData
{
struct DataToLog
{
    int first;
    double second;
    std::string third;
};

std::ostream& operator<<(std::ostream& os, const DataToLog& data)
{
    os << data.first << "," << data.second << "," << data.third;
    return os;
}
}

namespace LoggingFacility
{
template <typename DataType>
class Logger
{
public:
    Logger(std::string header, std::ostream& output, size_t bufferSize=1)
        :
          worker(&Logger<DataType>::ProcessBuffer, this)
        , outputStream(output)
        , bufferSize{bufferSize}
    {
        outputStream << header << std::endl;
    }

    Logger<DataType>& operator<<(DataType data)
    {
        {
            std::lock_guard<std::mutex> lk(bufferLock);
            buffer.push_back(std::move(data));
        }
        if(buffer.size() >= bufferSize)
        {
            bufferFilledCondition.notify_one();
        }
        return *this;
    }
    ~Logger()
    {
        continueWork.store(false);
        bufferFilledCondition.notify_one();
        worker.join();
    }
private:
    void ProcessBuffer()
    {
        while(continueWork.load())
        {
            std::unique_lock<std::mutex> lk(bufferLock);
            bufferFilledCondition.wait(lk, [this]{return !buffer.empty() || !continueWork.load();});
            std::swap(buffer, doubleBuffer);
            lk.unlock();
            for(const auto& element: doubleBuffer)
            {
                outputStream << element << '\n';
            }
            outputStream << std::flush;
            doubleBuffer.clear();
        }
    }

    std::atomic_bool continueWork {true};
    std::mutex bufferLock;
    std::condition_variable bufferFilledCondition;
    std::vector<DataType> buffer;
    std::vector<DataType> doubleBuffer;
    std::thread worker;
    std::ostream& outputStream;
    size_t bufferSize;
};
}

int main()
{
    using UserData::DataToLog;
    std::string header("first,second,third");
    std::ofstream fs("testLog.csv");
    LoggingFacility::Logger<DataToLog> logger(header, fs);
    logger << DataToLog{1, 2.0, "Third"}
           << DataToLog{2, 4.0, "h"};
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    logger << DataToLog{3, 8.0, "lk"}
           << DataToLog{4, 16.0, "hello"};
    return 0;
}
