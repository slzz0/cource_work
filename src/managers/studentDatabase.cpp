#include "managers/studentDatabase.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <ctime>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

#include "exceptions/exceptions.h"

void StudentDatabase::addStudent(std::shared_ptr<Student> student) {
    if (student != nullptr) {
        students.push_back(student);
    }
}

void StudentDatabase::addStudent(std::string_view name, std::string_view surname, int course,
                                 int semester, double averageGrade, bool isBudget) {
    for (const auto& existing : students) {
        if (existing->getName() == name && existing->getSurname() == surname) {
            throw DuplicateStudentException(std::format("{} {}", surname, name));
        }
    }

    auto student =
        std::make_shared<Student>(name, surname, course, semester, averageGrade, isBudget);
    students.push_back(student);
}

bool StudentDatabase::removeStudent(std::string_view name, std::string_view surname) {
    auto [first, last] = std::ranges::remove_if(students,
                                                [&name, &surname](const std::shared_ptr<Student>& student) {
                                                    return student->getName() == name && student->getSurname() == surname;
                                                });

    if (first != last) {
        students.erase(first, last);
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
    if (auto it = std::ranges::find(students, studentPtr); it != students.end()) {
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

std::string toLower(std::string_view str) {
    std::string result(str);
    std::ranges::transform(result, result.begin(),
                           [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByName(std::string_view name) const {
    std::string lowerName = toLower(name);
    return searchStudents([&lowerName](const std::shared_ptr<Student>& student) {
        return toLower(student->getName()).contains(lowerName) ||
               toLower(student->getFullName()).contains(lowerName);
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchBySurname(
    std::string_view surname) const {
    std::string lowerSurname = toLower(surname);
    return searchStudents([&lowerSurname](const std::shared_ptr<Student>& student) {
        return toLower(student->getSurname()).contains(lowerSurname);
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageGrade(double minGrade,
                                                                            double maxGrade) const {
    return searchStudents([minGrade, maxGrade](const std::shared_ptr<Student>& student) {
        double avg = student->calculateAverageGrade();
        return avg >= minGrade && avg <= maxGrade;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageAtLeast(
    double minGrade) const {
    return searchStudents([minGrade](const std::shared_ptr<Student>& student) {
        return student->calculateAverageGrade() >= minGrade;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageEqual(
    double exactGrade) const {
    auto round2 = [](double v) { return std::round(v * 100.0) / 100.0; };
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

void StudentDatabase::clear() { students.clear(); }

bool StudentDatabase::saveToFile(std::string_view fname) const {
    std::string actualFilename = fname.empty() ? filename : std::string(fname);
    std::ofstream file(actualFilename);
    if (!file.is_open()) {
        throw FileWriteException(actualFilename);
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // Use std::chrono for thread-safe date formatting
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t);
#else
    localtime_r(&time_t, &tm_buf);
#endif
    std::stringstream dateStream;
    dateStream << std::put_time(&tm_buf, "%Y-%m-%d");

    file << "# Scholarship Management System - Student Database\n";
    file << "# Generated: " << dateStream.str() << "\n";
    file << "# Total Students: " << students.size() << "\n\n";

    for (size_t i = 0; i < students.size(); ++i) {
        const auto& s = students[i];
        file << "[" << (i + 1) << "]\n";
        file << "Name:        " << s->getName() << "\n";
        file << "Surname:     " << s->getSurname() << "\n";
        file << "Semester:    " << s->getSemester() << "\n";
        file << "Avg Grade:   " << std::fixed << std::setprecision(2) << s->getAverageGrade()
             << "\n";
        file << "Funding:     " << (s->getIsBudget() ? "Budget" : "Paid") << "\n";
        file << "Missed Hours: " << s->getMissedHours() << "\n";
        file << "Social:      " << (s->getHasSocialScholarship() ? "Yes" : "No") << "\n";
        std::string history = s->getHistoryString();
        file << "Previous:    " << (history.empty() ? "" : history) << "\n";
        if (i < students.size() - 1) file << "\n---\n\n";
    }
    file << "\n# End of file\n";
    file.close();
    return true;
}

bool StudentDatabase::loadFromFile(std::string_view fname) {
    std::string actualFilename = fname.empty() ? filename : std::string(fname);
    std::ifstream file(actualFilename);
    if (!file.is_open()) return false;

    students.clear();

    std::string line;
    std::string name, surname, funding, social, previous;
    int course = 0, semester = 0, missedHours = 0;
    double avgGrade = 0.0, scholarship = 0.0;
    bool inBlock = false;
    std::vector<std::string> errors; 

    while (std::getline(file, line)) {
        if (line.find('[') == 0 && line.find(']') != std::string::npos) {
            if (inBlock && !name.empty() && !surname.empty()) {
                try {
                    bool isBudget = (funding == "Budget");
                    auto student = std::make_shared<Student>(name, surname, course, semester,
                                                             avgGrade, isBudget);
                    student->setMissedHours(missedHours);
                    student->setHasSocialScholarship(social == "Yes");

                    if (!previous.empty()) {
                        std::istringstream iss(previous);
                        std::string token;
                        while (std::getline(iss, token, ';')) {
                            size_t colon = token.find(':');
                            if (colon != std::string::npos) {
                                int sem = std::stoi(token.substr(0, colon));
                                double grade = std::stod(token.substr(colon + 1));
                                student->addPreviousGrade(sem, grade);
                            }
                        }
                    }

                    students.push_back(student);
                } catch (const ValidationException& e) {
                    errors.push_back(std::string("Student: ") + surname + " " + name + " - " +
                                     e.what());
                } catch (const std::exception& e) {
                    errors.push_back(std::string("Student: ") + surname + " " + name + " - " +
                                     e.what());
                }
            }

            name.clear();
            surname.clear();
            funding.clear();
            social.clear();
            previous.clear();
            course = 0;
            semester = 0;
            missedHours = 0;
            avgGrade = 0.0;
            inBlock = true;
            continue;
        }

        if (!inBlock) continue;

        if (line.find("Name:") == 0) {
            name = line.substr(line.find(":") + 1);
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
        } else if (line.find("Surname:") == 0) {
            surname = line.substr(line.find(":") + 1);
            surname.erase(0, surname.find_first_not_of(" \t"));
            surname.erase(surname.find_last_not_of(" \t\r\n") + 1);
        } else if (line.find("Course:") == 0) {
        } else if (line.find("Semester:") == 0) {
            std::sscanf(line.c_str(), "Semester: %d", &semester);
            course = (semester - 1) / 2 + 1;
        } else if (line.find("Avg Grade:") == 0) {
            std::sscanf(line.c_str(), "Avg Grade: %lf", &avgGrade);
        } else if (line.find("Funding:") == 0) {
            funding = line.substr(line.find(":") + 1);
            funding.erase(0, funding.find_first_not_of(" \t"));
            funding.erase(funding.find_last_not_of(" \t\r\n") + 1);
        } else if (line.find("Missed Hours:") == 0) {
            std::sscanf(line.c_str(), "Missed Hours: %d", &missedHours);
        } else if (line.find("Social:") == 0) {
            social = line.substr(line.find(":") + 1);
            social.erase(0, social.find_first_not_of(" \t"));
            social.erase(social.find_last_not_of(" \t\r\n") + 1);
        } else if (line.find("Previous:") == 0) {
            previous = line.substr(line.find(":") + 1);
            previous.erase(0, previous.find_first_not_of(" \t"));
            previous.erase(previous.find_last_not_of(" \t\r\n") + 1);
        }
        else if (line.find("---") == 0 || line.find("# End") == 0) {
        }
    }

    if (inBlock && !name.empty() && !surname.empty()) {
        try {
            bool isBudget = (funding == "Budget");
            auto student =
                std::make_shared<Student>(name, surname, course, semester, avgGrade, isBudget);
            student->setMissedHours(missedHours);
            student->setHasSocialScholarship(social == "Yes");

            if (!previous.empty()) {
                std::istringstream iss(previous);
                std::string token;
                while (std::getline(iss, token, ';')) {
                    size_t colon = token.find(':');
                    if (colon != std::string::npos) {
                        int sem = std::stoi(token.substr(0, colon));
                        double grade = std::stod(token.substr(colon + 1));
                        student->addPreviousGrade(sem, grade);
                    }
                }
            }

            students.push_back(student);
        } catch (const ValidationException& e) {
            errors.push_back(std::string("Student: ") + surname + " " + name + " - " + e.what());
        } catch (const std::exception& e) {
            errors.push_back(std::string("Student: ") + surname + " " + name + " - " + e.what());
        }
    }

    file.close();

    if (!errors.empty()) {
        std::string errorMsg = "Some students were skipped due to invalid data:\n";
        for (size_t i = 0; i < errors.size(); ++i) {
            errorMsg += errors[i];
            if (i < errors.size() - 1) errorMsg += "\n";
        }
        throw ParseException(errorMsg);
    }

    return !students.empty();
}