#ifndef SCHOLARSHIPCALCULATOR_H
#define SCHOLARSHIPCALCULATOR_H

#include <map>

#include "student.h"

class ScholarshipCalculator {
   public:
    static constexpr double SCHOLARSHIP_5_0_5_9 = 157.18;
    static constexpr double SCHOLARSHIP_6_0_7_9 = 188.61;
    static constexpr double SCHOLARSHIP_8_0_8_9 = 220.05;
    static constexpr double SCHOLARSHIP_9_0_10_0 = 251.48;
    static constexpr double SOCIAL_SCHOLARSHIP = 100.00;

    static double calculateScholarship(double averageGrade);
    static double calculateScholarshipForStudent(const Student* student);
    static std::string getScholarshipBracket(double averageGrade);
};

#endif