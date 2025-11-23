#ifndef SCHOLARSHIP_H
#define SCHOLARSHIP_H

#include <string>

class Scholarship {
public:
    Scholarship(double amount = 0.0, bool isSocial = false, int semester = 0);
    
    double getAmount() const { return amount; }
    void setAmount(double value) { amount = value; }
    
    bool getIsSocial() const { return isSocial; }
    void setIsSocial(bool value) { isSocial = value; }
    
    int getSemester() const { return semester; }
    void setSemester(int value) { semester = value; }
    
    // Static constants for scholarship amounts
    static constexpr double SCHOLARSHIP_5_0_5_9 = 157.18;
    static constexpr double SCHOLARSHIP_6_0_7_9 = 188.61;
    static constexpr double SCHOLARSHIP_8_0_8_9 = 220.05;
    static constexpr double SCHOLARSHIP_9_0_10_0 = 251.48;
    static constexpr double SOCIAL_SCHOLARSHIP = 100.00;
    
    // Static methods for calculation
    static double calculateScholarshipAmount(double averageGrade);
    static std::string getScholarshipBracket(double averageGrade);
    
private:
    double amount;
    bool isSocial;
    int semester;
};

#endif // SCHOLARSHIP_H

