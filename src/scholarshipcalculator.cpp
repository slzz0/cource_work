#include "scholarshipCalculator.h"
#include <sstream>
#include <iomanip>

double ScholarshipCalculator::calculateScholarship(double averageGrade) {
    if (averageGrade >= 9.0 && averageGrade <= 10.0) {
        return SCHOLARSHIP_9_0_10_0;
    } else if (averageGrade >= 8.0 && averageGrade < 9.0) {
        return SCHOLARSHIP_8_0_8_9;
    } else if (averageGrade >= 6.0 && averageGrade < 8.0) {
        return SCHOLARSHIP_6_0_7_9;
    } else if (averageGrade >= 5.0 && averageGrade < 6.0) {
        return SCHOLARSHIP_5_0_5_9;
    }
    return 0.0; // No scholarship for grades below 5.0
}

double ScholarshipCalculator::calculateScholarshipForStudent(const Student* student) {
    if (student == nullptr) {
        return 0.0;
    }
    return calculateScholarship(student->calculateAverageGrade());
}

std::string ScholarshipCalculator::getScholarshipBracket(double averageGrade) {
    if (averageGrade >= 9.0 && averageGrade <= 10.0) {
        return "9.0 - 10.0";
    } else if (averageGrade >= 8.0 && averageGrade < 9.0) {
        return "8.0 - 8.9";
    } else if (averageGrade >= 6.0 && averageGrade < 8.0) {
        return "6.0 - 7.9";
    } else if (averageGrade >= 5.0 && averageGrade < 6.0) {
        return "5.0 - 5.9";
    }
    return "No scholarship";
}
