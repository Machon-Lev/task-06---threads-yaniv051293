#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>

struct Message {
    int tid;
	double content;
	bool end;
};

std::queue<Message> messageQueue; // Queue to hold the messages
std::mutex mtx; // Mutex for protecting the message queue
std::condition_variable cv; // Condition variable for signaling new messages



void consumerFunction() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until there is a message in the queue
        cv.wait(lock, [] { return !messageQueue.empty(); });


        // Retrieve the message from the queue
        try 
        {
            Message message = messageQueue.front();
            messageQueue.pop();
            lock.unlock();
            if (message.end == false)
                std::cout << message.tid << " sent: " << message.content << "\n";
            else
                std::cout << message.tid << " finished\n";
        }
        catch(const std::exception& e){
            std::cout << "Exception caught: " << e.what() << std::endl;
        }


    
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }
}

void producerFunction() {

    int tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
    double number = tid;
    while (number >= 1)
    {
        Message message;
        message.tid = tid;
        message.content = number;
        message.end = number / 10 < 1;

        {
            std::lock_guard<std::mutex> lock(mtx);
            messageQueue.push(message);
        }
        cv.notify_one(); // Notify the receiving thread about the new message

        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        number = number / 10;
    }


}

int main()
{
	std::thread consumerThread(consumerFunction);
	std::thread firstProducerThread(producerFunction);
	std::thread secProducerThread(producerFunction);



	firstProducerThread.join();
    secProducerThread.join();

    cv.notify_one();

    consumerThread.join();
    return 0;
}


