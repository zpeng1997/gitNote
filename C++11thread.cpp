#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
 
std::mutex data_mutex;
std::condition_variable data_var;
int flag = 1;
 
void printA()
{
    int n = 5;
    while(-- n)
    {
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lck(data_mutex) ;
        data_var.wait(lck,[]{return flag == 1;});
        std::cout<<"thread: "<< std::this_thread::get_id() << "   printf: " << "A" <<std::endl;
        flag = 2;
        // lck.unlock();
        data_var.notify_all();
    }
}
 
void printB()
{   
    int n = 5;
    while(-- n)
    {
        std::unique_lock<std::mutex> lck(data_mutex) ;
        data_var.wait(lck,[]{return flag == 2;});
        std::cout<<"thread: "<< std::this_thread::get_id() << "   printf: " << "B" <<std::endl;
        flag = 3;
        data_var.notify_all();
    }
}
 
void printC()
{
    int n = 5;
    while(-- n)
    {
        std::unique_lock<std::mutex> lck(data_mutex);
        data_var.wait(lck,[]{return flag == 3;});
        std::cout <<"thread: " << std::this_thread::get_id() << " printf: " << "C" << std::endl;
        flag = 1;
        data_var.notify_all();
    }
}

int main()
{
    std::thread tA(printA);
    std::thread tB(printB);
    std::thread tC(printC);
    tA.join();
    tB.join();
    tC.join();
    return 0;
}