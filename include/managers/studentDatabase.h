#ifndef STUDENTDATABASE_H
#define STUDENTDATABASE_H

#include <algorithm>
#include <functional>
#include <memory>
#include <ranges>
#include <string_view>
#include <vector>

#include "entities/student.h"


class StudentDatabase {
   private:
    std::vector<std::shared_ptr<Student>> students;
    std::string filename = "students.txt";

   public:
    StudentDatabase() = default;
    ~StudentDatabase() = default;

    void addStudent(std::shared_ptr<Student> student);
    void addStudent(std::string_view name, std::string_view surname, int course, int semester,
                    double averageGrade, bool isBudget);

    bool removeStudent(std::string_view name, std::string_view surname);
    bool removeStudent(size_t index);
    bool removeStudentPtr(const std::shared_ptr<Student>& studentPtr);

    std::vector<std::shared_ptr<Student>> getAllStudents() const { return students; }
    size_t getStudentCount() const { return students.size(); }
    std::shared_ptr<Student> getStudent(size_t index) const;

    template <typename Predicate>
    std::vector<std::shared_ptr<Student>> searchStudents(Predicate predicate) const {
        std::vector<std::shared_ptr<Student>> results;
        std::ranges::copy_if(students, std::back_inserter(results), predicate);
        return results;
    }

    std::vector<std::shared_ptr<Student>> searchByName(std::string_view name) const;
    std::vector<std::shared_ptr<Student>> searchBySurname(std::string_view surname) const;
    std::vector<std::shared_ptr<Student>> searchByAverageGrade(double minGrade,
                                                               double maxGrade) const;
    std::vector<std::shared_ptr<Student>> searchByAverageAtLeast(double minGrade) const;
    std::vector<std::shared_ptr<Student>> searchByAverageEqual(double exactGrade) const;
    std::vector<std::shared_ptr<Student>> searchByCourse(int course) const;

    bool saveToFile(std::string_view filename = "") const;
    bool loadFromFile(std::string_view filename = "");
    void setFilename(std::string_view newFilename) { filename = newFilename; }

    void clear();
};

#endif