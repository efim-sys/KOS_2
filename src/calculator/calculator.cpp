#include<vector>
#include<cstring>
#include<utility>
#include<cmath>
#include <string>

using namespace std; //used for improved code readability, very simple code
typedef double equi_return_type; 
typedef ssize_t equi_length; //number of characters in equi string

constexpr size_t MAX_LENGTH{ 10 };
constexpr size_t MAX_EQ_CHARS{ 100 };
char* FindNumber(char* equi, int& it);
char* ReverseNumber(char* number, size_t length);  
bool Validate(char* number, size_t length); 
void Multiply(vector<double>&  numbers, vector<pair<int, int>>&  mltp);
equi_return_type Add(vector<double>&  numbers);
equi_return_type Solve(char* number,int& it,bool tc=0);
equi_return_type Result(vector<double>&  numbers, vector<pair<int, int>>&  mltp);

// int main()
// {
// 	char equi[MAX_EQ_CHARS];
// 	cin>>equi;
// 	int length=strlen(equi)-1;
// 	cout << Solve(equi, length);
// }

char* FindNumber(char* equi, int& it) //stores characters in string until it encounters digit,bracket or something else
{									  // function Solve handles what happens with character after this function ends, so if next character is something
	auto number{new char[MAX_LENGTH]};  // unexpected, NaN will be thrown
	auto i{0};
	while (equi[it]>='0'&&equi[it]<='9'||equi[it]=='.')
	{
		number[i++] = equi[it--];
	}
	number[i] = '\0';
	return number;
}

char* ReverseNumber(char* number, size_t length)  //as written in documentation, we are going from last character in equi to first, so we can know
{												  //in place what we can do with operators we encounter
	auto reversed = new char[length + 1];
	if (!reversed)
		return nullptr;

	reversed[length] = '\0';
	for (auto i{ 0 }; i < length; ++i) {
		reversed[i] = number[length - 1 - i];
	}
	free(number);
	return reversed;
}

bool Validate(char* number, size_t length)
{
	if (number[0] < 48 || number[0]>57 || number[length - 1] < 48 || number[length - 1]>57)  //checks if first and last characters of number are digits
		return 0;

	if (length == 2) //if previous "if" is met, we know there is no decimal comma, so we don't need to check if comma is on right place		
	return 1;

	auto dcn{ 0 };
	for (auto i{ 1 }; i < length - 1; ++i) //checks if there are more than 1 decimal comma and if it is on right place
	{
		if (number[i] == '.')
			++dcn;

		if (number[i]!='.'&&(number[i] < '0' || number[i]>'9'))
			return 0;
	}
	if (dcn > 1)
		return 0;

	return 1;

}

void Multiply(vector<double>& numbers, vector<pair<int, int>>& mltp) // indexes of numbers we need to multiply
{																	// they also can be hold in array, but this is simpler way to do

	if (mltp.size() == 0)
		return;

	for (int i = mltp.size()-1; i >=0; --i)
	{
		numbers[mltp[i].first] = numbers[mltp[i].second] * numbers[mltp[i].first];
		numbers[mltp[i].second] = 0;
	}
}

equi_return_type Add(vector<double>& numbers)	//adds all numbers in vector where we store them
{
			double result{0};
			for(auto i{0};i<numbers.size();++i)
			{
				result+=numbers[i];
				numbers.at(i)=0;
			}			
			return result;
}

equi_return_type Result(vector<double>& numbers, vector<pair<int, int>>& mltp) 
{
	Multiply(numbers, mltp);
	return Add(numbers);	
}

double Solve(char* equi,int& it, bool tc) {

	vector<double> numbers;
	vector<pair<int, int>> mltp;
	int index{0};
		
	while(it>=0)
	{
		if(equi[it]==')')
		{
			numbers.push_back(Solve(equi,--it,1));
			if(equi[it]=='+');
			
			else if(equi[it]=='-'){
				numbers[index]=0-numbers[index];
			}
			
			else if(equi[it]=='/'){
				numbers[index]=1/numbers[index];
				mltp.push_back(make_pair(index,index+1));
			}
			
			else if(equi[it]=='*')
			mltp.push_back(make_pair(index,index+1));
			
			else if(equi[it]=='('&&tc)
		{
			it--;
			return Result(numbers,mltp);
			
		}
						
			index++;
			it--;			
		}
		
		if(equi[it]=='('&&tc)
		{
			it--;
			return Result(numbers,mltp);			
		}
				
		char* number{FindNumber(equi,it)};
		number=ReverseNumber(number,strlen(number));
		
		double checked;
		if(!Validate(number,strlen(number)))
		{
			return nan("");
		}
		checked=stod(number);
			
			if(equi[it]=='+')
			numbers.push_back(checked);
			
			else if(equi[it]=='-')
			numbers.push_back(0-checked);
			
			else if(equi[it]=='/'){
				numbers.push_back(1/checked);
			mltp.push_back(make_pair(index,index+1));
			}
			
			else if(equi[it]=='*'){
				numbers.push_back(checked);
			mltp.push_back(make_pair(index,index+1));
			}
			
			else if(it<0){
			numbers.push_back(checked);
			}
			
			else if(equi[it]=='('&&tc){
				numbers.push_back(checked);
				it++;
			}
			
			else
			return nan("");

			
			index++;
			it--;
	
	}

	return Result(numbers,mltp);	
}
