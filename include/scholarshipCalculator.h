#ifndef SCHOLARSHIPCALCULATOR_H
#define SCHOLARSHIPCALCULATOR_H

#include "student.h"
#include <map>

/**
 * @brief Class for calculating scholarships based on average grades
 */
class ScholarshipCalculator {
public:
    // Scholarship amounts in BYN
    static constexpr double SCHOLARSHIP_5_0_5_9 = 157.18;
    static constexpr double SCHOLARSHIP_6_0_7_9 = 188.61;
    static constexpr double SCHOLARSHIP_8_0_8_9 = 220.05;
    static constexpr double SCHOLARSHIP_9_0_10_0 = 251.48;

    /**
     * @brief Calculate scholarship amount based on average grade
     * @param averageGrade Average grade of the student
     * @return Scholarship amount in BYN
     */
    static double calculateScholarship(double averageGrade);
    
    /**
     * @brief Calculate scholarship for a student
     * @param student Pointer to student object
     * @return Scholarship amount in BYN
     */
    static double calculateScholarshipForStudent(const Student* student);
    
    /**
     * @brief Get scholarship bracket information
     * @param averageGrade Average grade of the student
     * @return String describing the scholarship bracket
     */
    static std::string getScholarshipBracket(double averageGrade);
};

#endif // SCHOLARSHIPCALCULATOR_H

