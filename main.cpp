#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <yarp/os/all.h>
#include <iCub/emorph/vCodec.h>
#include <iCub/emorph/all.h>
#include <yarp/sig/all.h>
#include <yarp/name/all.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Time.h>

using namespace std;
using namespace emorph;
using namespace yarp::os;


using namespace std;
std::string filename_in= "/home/yeshi/AER_files/data/speed_fast.txt";
std::ifstream infile;
std::string line;

std::string filename_out= "/home/yeshi/AER_files/data/data.log";
std::ofstream outfile;

std::string filename_info_out= "/home/yeshi/AER_files/data/info.log";
std::ofstream outfile_info;



//Variables
int channel;
int stamp;
int polarity;
int x;
int y;
std::string type="AE";

std::vector<double> yarp_stamp;
std::vector<double>::iterator yarp_stamp_it;
std::vector<double>::reverse_iterator yarp_stamp_rev_it;
double timeStamp=0;

std::vector<double> time_stamp;
std::vector<double>::iterator time_stamp_it;
std::vector<double>::reverse_iterator time_stamp_rev_it;


double time_sec = 0.0000001; //Converting to sec
double time_ms = 1000; //Converting to ms
double time_diff=0;

//double t1=yarp::os::Time::now();

void write(int word0, int word1){

        outfile << word0 << " " << word1 << " ";

}


int main()
{

    infile.open(filename_in.c_str(),ios::in); //NOTE Check the arguments
    outfile.open(filename_out.c_str(),ios::out|ios::trunc);//Close and delete the contents on each run
    if(outfile.is_open()){
        //std::cout << "Closing the output file..." << std::endl;
        outfile.close();
    }

    outfile_info.open(filename_info_out.c_str(),ios::out|ios::trunc);//Writing the begining of the file
    if(outfile_info.is_open()){
        //std::cout << "Info file opened..."<<std::endl;//Debug Code
        outfile_info<< "Type: Bottle;" << "\n";
        outfile_info.close();
    }


    if(infile.is_open()){
        //std::cout << "Input text file is open for reading..." << std::endl;
        while(!infile.eof()){ //TODO When the input file is open pass each row and convert to vBottles

            //TODO read the file for one ms of timestamps and put them in an AE vBottle
            while(std::getline(infile,line)){ //

                std::stringstream   linestream(line);
                //std::cout << "Reading the input text file..." << std::endl;//Debug Code
                linestream >> channel >> stamp >>polarity >> x >> y;
                //std::cout << channel << " " << stamp << " " << polarity<< " "  << x<< " "  << y << " ";//Debug Code

                //Encoding
                int word0=(32<<26)|(stamp&0x00ffffff);
                int word1=(0<<26)|((channel&0x01)<<15)|((y&0x7f)<<8)|((x&0x7f)<<1)| (polarity&0x01);
                //std::cout << word0 << " " << word1 <<std::endl; //Debug Code

                //TODO Fix the time stamps
                /*double ts =  stamp/time_sec;
                std::cout << "Time stamps Int : " << ts << std::endl;//Debug Code
                double td =  stamp%time_sec;
                std::cout << "Time stamps Decimal : " << td << std::endl;//Debug Code*/
                timeStamp = stamp*time_sec;
                //std::cout << "Time stamps in Seconds : " << timeStamp << std::endl;//Debug Code
                if(!outfile.is_open()){//Check if the file is open already, if not open it and start writing

                    //std::cout << "The output file is closed..." << std::endl;
                    outfile.open(filename_out.c_str(),ios::out|ios::app);
                    if(outfile.is_open()){
                        //std::cout << "Opening the output file..." << std::endl;
                        outfile << -1 << " " << timeStamp << " " << type << " " << '(';
                    }

                }

                //Pushing event time stamps into vector
                yarp_stamp.push_back(stamp);
                yarp_stamp_it = yarp_stamp.begin();
                yarp_stamp_rev_it = yarp_stamp.rbegin(); //This changes as the elements are added to the vector

                //Pushing all the time stamps into a vector
                time_stamp.push_back(timeStamp);


                //Finding the difference between last and first timestamp
                time_diff = *yarp_stamp_rev_it - *yarp_stamp_it;
                time_diff = time_diff/time_ms; //Converting to ms
                //std::cout << "First Element : " << *yarp_stamp_it << "Last Element : " << *yarp_stamp_rev_it << "Time Difference : " << time_diff << std::endl;


                if(time_diff <=1){
                    write(word0,word1);//Sending the encoded data for each event
                }

                else{ //If time difference is more than 1ms reset difference and clear the stamps vectors
                    time_diff=0;
                    yarp_stamp.clear();
                    outfile<< "\b" <<')' << "\n";
                    //std::cout << "Closing the output file..." << std::endl;
                    outfile.close(); //If the vBottle is ready close the file
                }

            }

        }
        std::cout << "Reached EOF of input file..." << std::endl;
        outfile<< "\b" <<')' << "\n";
        outfile.close();

    }
     else {std::cout<<"Unable to read the input text file!!!"<<std::endl;}


    if(outfile.is_open()){
        std::cout << "Closing the output file..." << std::endl;
        outfile.close();
    }

    time_stamp_it = time_stamp.begin();
    time_stamp_rev_it = time_stamp.rbegin();
    outfile_info.open(filename_info_out.c_str(),ios::out|ios::app);//Writing the begining of the file
    if(outfile_info.is_open()){
        outfile_info<< "[" << *time_stamp_it << "]" <<" /aexGrabber/vBottle:o [connected]" << "\n";
        outfile_info<< "[" << *time_stamp_rev_it << "]" <<" /aexGrabber/vBottle:o [disconnected]" << "\n";
        outfile_info.close();
        time_stamp.clear();//Clearing the time stamps vector
    }

    infile.close();
    return 0;
}




