#include <iostream> 
#include <string> 
#include <stdexcept> 
#include <vector> 
#include <thread> 
#include <mutex> 
#include <condition_variable> 

// Enum for UART configuration parameters 
enum class UARTConfig { BAUD_RATE, DATA_BITS, PARITY, STOP_BITS }; 

// Enum for possible UART states 
enum class UARTState { CLOSED, OPEN, CONFIGURED }; 

// Enum for communication methods 
enum class CommMethod { POLLING, INTERRUPT, DMA }; 

// UART class definition 
class UART 
{ 
    public: UART(int id); 
    ~UART(); 
    void configure(UARTConfig config, int value); 
    void open(CommMethod method); 
    void close(); 
    int read(char* buffer, int length); 
    int write(const char* data, int length); 
    private: int uartId; // UART identifier 
    UARTState state;  
    int baudRate;  
    int dataBits;  
    bool parity;  
    int stopBits; 
    CommMethod commMethod; //(polling, interrupt or DMA) 
    // Synchronization primitives for asynchronous operations 
    std::mutex mtx; 
    std::condition_variable cv; 
    bool dataReady; // Flag to indicate if data is ready for reading 
    std::vector<char> readBuffer; // Buffer to store read data 
    void asyncRead(); // Function for asynchronous reading using interrupts 
    void dmaRead(); // Function for asynchronous reading using DMA 
    }; 
    // Constructor 
    UART::UART(int id) : uartId(id), state(UARTState::CLOSED), baudRate(9600), dataBits(8), parity(false), stopBits(1), dataReady(false) 
    { 
        // Initialization of UART with default values     
    } 
    // Destructor 
    UART::~UART() 
    { 
        if (state == UARTState::OPEN) 
        { 
            close(); // Ensure UART is closed upon destruction, will prevent errors in the next read cycle
        } 
    } 
    // Configure the UART 
    void UART::configure(UARTConfig config, int value) 
    { 
        // Lock thread for safety
        std::lock_guard<std::mutex> lock(mtx); 
         
        if (state != UARTState::CLOSED) 
        { 
            throw std::runtime_error("UART must be closed before configuration.");             
        } 
        
        switch (config) 
        { 
            case UARTConfig::BAUD_RATE: baudRate = value; 
            break; 
            case UARTConfig::DATA_BITS: dataBits = value; 
            break; 
            case UARTConfig::PARITY: parity = (value != 0); 
            break; 
            case UARTConfig::STOP_BITS: stopBits = value; 
            break; 
            default: 
            throw std::invalid_argument("Invalid UART configuration."); 
        } 
        state = UARTState::CONFIGURED;  
        
    } 
        // Open UART, pass method into constructor
        void UART::open(CommMethod method) 
        { 
            std::lock_guard<std::mutex> lock(mtx); // Ensure thread safety 
            if (state == UARTState::OPEN) 
            { 
                throw std::runtime_error("UART is already open."); 
            } 
            if (state != UARTState::CONFIGURED) 
            { 
                throw std::runtime_error("UART must be configured before opening."); 
            } 
            
            //TODO - hardware interaction
            state = UARTState::OPEN; 
            commMethod = method; 
            // Start appropriate asynchronous read method based on communication method 
            if (method == CommMethod::INTERRUPT) 
            { 
                std::thread(&UART::asyncRead, this).detach(); 
            } 
            else if (method == CommMethod::DMA) 
            { 
                std::thread(&UART::dmaRead, this).detach(); 
            } 
        } 
        
        void UART::close() 
        { 
            std::lock_guard<std::mutex> lock(mtx); 
            if (state != UARTState::OPEN) 
            { 
                throw std::runtime_error("UART is not open."); 
            } 
            //TODO - hardware interaction
            state = UARTState::CLOSED; 
            
        } 
        // Read data from the UART 
            int UART::read(char* buffer, int length) 
            { 
                std::unique_lock<std::mutex> lock(mtx); 
            
            if (state != UARTState::OPEN) 
            { 
                throw std::runtime_error("UART is not open."); 
            }
                
                 
                 if (commMethod == CommMethod::POLLING) 
                 { 
                    // TODO - Polling read implementation, read register/peripheral in a loop with some delay built in
                 } 
                 // For interrupt or DMA methods wait for data to become available
                 cv.wait(lock, [this] { return dataReady; }); 
                 
                 // Determine how many bytes to read 
                 int bytesRead = std::min(length, static_cast<int>(readBuffer.size())); 
                 std::copy(readBuffer.begin(), readBuffer.begin() + bytesRead, buffer); 
                 readBuffer.erase(readBuffer.begin(), readBuffer.begin() + bytesRead); 
                 dataReady = false; 
                 return bytesRead; // Return the number of bytes read 
        } 
        
        
        int UART::write(const char* data, int length) 
        { 
            std::lock_guard<std::mutex> lock(mtx); 
            
            if (state != UARTState::OPEN) 
            { 
                throw std::runtime_error("UART is not open."); 
            } 
            //TODO - hardware interaction
        } 
            
            void UART::asyncRead() 
            { 
                while (state == UARTState::OPEN) 
                { 
                //TODO - hardware interaction
                
                //Wait for data from hardware               
                cv.notify_one(); 
                // Notify the read function when data is available
                }
            } 
            
        } 
        // Asynchronous read function using DMA 
        void UART::dmaRead() 
        { 
            while (state == UARTState::OPEN) 
                { 
                //TODO - hardware interaction
                
                //Wait for data from hardware               
                cv.notify_one(); 
                // Notify the read function when data is available
                }
        } 
        
        int main() 
        { 
            UART uart(1); // Create UART instance with ID 1 
            // Example usage of UART API 
            try 
            { 
                uart.configure(UARTConfig::BAUD_RATE, 115200); 
                uart.configure(UARTConfig::DATA_BITS, 8); 
                uart.configure(UARTConfig::PARITY, 0); 
                uart.configure(UARTConfig::STOP_BITS, 1); 
                uart.open(CommMethod::INTERRUPT); 
                // Open with interrupt method 
                char buffer[10]; 
                int bytesRead = uart.read(buffer, sizeof(buffer)); 
                std::cout << "Read " << bytesRead << " bytes: " << std::string(buffer, bytesRead) << std::endl; 
                const char* data = "Hello, UART!"; 
                int bytesWritten = uart.write(data, strlen(data)); 
                std::cout << "Wrote " << bytesWritten << " bytes." << std::endl; 
                uart.close(); 
            } 
            catch (const std::exception& ex) 
            { 
                std::cerr << "Error: " << ex.what() << std::endl; 
            } 
            return 0; 
                
        }
