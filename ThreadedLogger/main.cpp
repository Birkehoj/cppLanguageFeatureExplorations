#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

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
    Logger(std::string header, std::ostream& output = std::cout)
        :
          worker(&Logger<DataType>::ProcessBuffer, this)
        , outputStream(output)
    {
        outputStream << header << std::endl;
    }

    Logger<DataType>& operator<<(DataType data)
    {
        {
            std::lock_guard<std::mutex> lk(bufferLock);
            buffer.push_back(std::move(data));
        }
        bufferFilledCondition.notify_one();
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
};
}

int main()
{
    using UserData::DataToLog;
    std::string header("first,second,third");
    LoggingFacility::Logger<DataToLog> logger(header);
    logger << DataToLog{1, 2.0, "Third"}
           << DataToLog{2, 4.0, "h"}
           << DataToLog{3, 8.0, "lk"}
           << DataToLog{4, 16.0, "hello"};
    return 0;
}
