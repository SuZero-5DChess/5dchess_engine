#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
    std::vector<int> xs = {3,8,7,5,6,2,4,1};
    int sign = -1;
    std::sort(xs.begin(), xs.end(), [sign=sign](int a, int b){
            return sign*a < sign*b;
        });
    for(int x:xs)
    {
        std::cout << x << ",";
    }
    std::cout << "\n\n";
}
