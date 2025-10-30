#include "studentDatabase.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <sstream>

void StudentDatabase::addStudent(std::shared_ptr<Student> student) {
    if (student != nullptr) {
        students.push_back(student);
    }
}

void StudentDatabase::addStudent(const std::string& name, const std::string& surname,
                                 int course, int semester, double averageGrade, bool isBudget) {
    auto student = std::make_shared<Student>(name, surname, course, semester, averageGrade, isBudget);
    students.push_back(student);
}

bool StudentDatabase::removeStudent(const std::string& name, const std::string& surname) {
    auto it = std::remove_if(students.begin(), students.end(),
        [&name, &surname](const std::shared_ptr<Student>& student) {
            return student->getName() == name && student->getSurname() == surname;
        });
    
    if (it != students.end()) {
        students.erase(it, students.end());
        return true;
    }
    return false;
}

bool StudentDatabase::removeStudent(size_t index) {
    if (index < students.size()) {
        students.erase(students.begin() + index);
        return true;
    }
    return false;
}

bool StudentDatabase::removeStudentPtr(const std::shared_ptr<Student>& studentPtr) {
    auto it = std::find(students.begin(), students.end(), studentPtr);
    if (it != students.end()) {
        students.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<Student> StudentDatabase::getStudent(size_t index) const {
    if (index < students.size()) {
        return students[index];
    }
    return nullptr;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByName(const std::string& name) const {
    std::string lowerName = toLower(name);
    return searchStudents([&lowerName](const std::shared_ptr<Student>& student) {
        return toLower(student->getName()).find(lowerName) != std::string::npos ||
               toLower(student->getFullName()).find(lowerName) != std::string::npos;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchBySurname(const std::string& surname) const {
    std::string lowerSurname = toLower(surname);
    return searchStudents([&lowerSurname](const std::shared_ptr<Student>& student) {
        return toLower(student->getSurname()).find(lowerSurname) != std::string::npos;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageGrade(double minGrade, double maxGrade) const {
    return searchStudents([minGrade, maxGrade](const std::shared_ptr<Student>& student) {
        double avg = student->calculateAverageGrade();
        return avg >= minGrade && avg <= maxGrade;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageAtLeast(double minGrade) const {
    return searchStudents([minGrade](const std::shared_ptr<Student>& student) {
        return student->calculateAverageGrade() >= minGrade;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageEqual(double exactGrade) const {
    auto round2 = [](double v) {
        return std::round(v * 100.0) / 100.0;
    };
    double target = round2(exactGrade);
    return searchStudents([target, round2](const std::shared_ptr<Student>& student) {
        return round2(student->calculateAverageGrade()) == target;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByCourse(int course) const {
    return searchStudents([course](const std::shared_ptr<Student>& student) {
        return student->getCourse() == course;
    });
}

void StudentDatabase::clear() {
    students.clear();
}

bool StudentDatabase::saveToFile(const std::string& fname) const {
    std::string actualFilename = fname.empty() ? filename : fname;
    std::ofstream file(actualFilename);
    if (!file.is_open()) return false;

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream dateStream;
    dateStream << std::put_time(std::localtime(&time_t), "%Y-%m-%d");

    file << "# Scholarship Management System - Student Database\n";
    file << "# Generated: " << dateStream.str() << "\n";
    file << "# Total Students: " << students.size() << "\n\n";

    for (size_t i = 0; i < students.size(); ++i) {
        const auto& s = students[i];
        file << "[" << (i + 1) << "]\n";
        file << "Name:        " << s->getName() << "\n";
        file << "Surname:     " << s->getSurname() << "\n";
        file << "Course:      " << s->getCourse() << "\n";
        file << "Semester:    " << s->getSemester() << "\n";
        file << "Avg Grade:   " << std::fixed << std::setprecision(2) << s->getAverageGrade() << "\n";
        file << "Funding:     " << (s->getIsBudget() ? "Budget" : "Paid") << "\n";
        file << "Scholarship: " << std::fixed << std::setprecision(2) << s->getScholarship() << " BYN\n";
        if (i < students.size() - 1) file << "\n---\n\n";
    }
    file << "\n# End of file\n";
    file.close();
    return true;
}

bool StudentDatabase::loadFromFile(const std::string& fname) {
    std::string actualFilename = fname.empty() ? filename : fname;
    std::ifstream file(actualFilename);
    if (!file.is_open()) return false;

    students.clear();

    std::string line;
    size_t totalStudents = 0;

    while (std::getline(file, line)) {
        if (line.find("# Total Students:") == 0) {
            std::sscanf(line.c_str(), "# Total Students: %zu", &totalStudents);
            break;
        }
    }

    if (totalStudents == 0) {
        file.close();
        return false; 
    }

    // Читаем студентов
    for (size_t i = 0; i < totalStudents; ++i) {
        std::string name, surname, funding;
        int course = 0, semester = 0;
        double avgGrade = 0.0, scholarship = 0.0;

        bool foundBlock = false;
        while (std::getline(file, line)) {
            if (line.find('[') == 0 && line.find(']') != std::string::npos) {
                foundBlock = true;
                continue;
            }
            if (!foundBlock) continue;

            if (line.find("Name:") == 0) {
                name = line.substr(line.find(":") + 1);
                name.erase(0, name.find_first_not_of(" \t"));
                name.erase(name.find_last_not_of(" \t\r\n") + 1);
            }
            else if (line.find("Surname:") == 0) {
                surname = line.substr(line.find(":") + 1);
                surname.erase(0, surname.find_first_not_of(" \t"));
                surname.erase(surname.find_last_not_of(" \t\r\n") + 1);
            }
            else if (line.find("Course:") == 0) {
                std::sscanf(line.c_str(), "Course: %d", &course);
            }
            else if (line.find("Semester:") == 0) {
                std::sscanf(line.c_str(), "Semester: %d", &semester);
            }
            else if (line.find("Avg Grade:") == 0) {
                std::sscanf(line.c_str(), "Avg Grade: %lf", &avgGrade);
            }
            else if (line.find("Funding:") == 0) {
                funding = line.substr(line.find(":") + 1);
                funding.erase(0, funding.find_first_not_of(" \t"));
                funding.erase(funding.find_last_not_of(" \t\r\n") + 1);
            }
            else if (line.find("Scholarship:") == 0) {
                std::sscanf(line.c_str(), "Scholarship: %lf", &scholarship);
            }
            else if (line.find("---") == 0 || line.find("# End") == 0) {
                break;
            }
        }

        if (name.empty() || surname.empty()) continue;

        bool isBudget = (funding == "Budget");
        auto student = std::make_shared<Student>(name, surname, course, semester, avgGrade, isBudget);
        student->setScholarship(scholarship);
        students.push_back(student);
    }

    file.close();
    return !students.empty();
}