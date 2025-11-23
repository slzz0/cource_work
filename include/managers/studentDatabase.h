#ifndef STUDENTDATABASE_H
#define STUDENTDATABASE_H

#include <algorithm>
#include <functional>
#include <memory>
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
    void addStudent(const std::string& name, const std::string& surname, int course, int semester,
                    double averageGrade, bool isBudget);

    bool removeStudent(const std::string& name, const std::string& surname);
    bool removeStudent(size_t index);
    bool removeStudentPtr(const std::shared_ptr<Student>& studentPtr);

    std::vector<std::shared_ptr<Student>> getAllStudents() const { return students; }
    size_t getStudentCount() const { return students.size(); }
    std::shared_ptr<Student> getStudent(size_t index) const;

    template <typename Predicate>
    std::vector<std::shared_ptr<Student>> searchStudents(Predicate predicate) const {
        std::vector<std::shared_ptr<Student>> results;
        std::copy_if(students.begin(), students.end(), std::back_inserter(results), predicate);
        return results;
    }

    std::vector<std::shared_ptr<Student>> searchByName(const std::string& name) const;
    std::vector<std::shared_ptr<Student>> searchBySurname(const std::string& surname) const;
    std::vector<std::shared_ptr<Student>> searchByAverageGrade(double minGrade,
                                                               double maxGrade) const;
    std::vector<std::shared_ptr<Student>> searchByAverageAtLeast(double minGrade) const;
    std::vector<std::shared_ptr<Student>> searchByAverageEqual(double exactGrade) const;
    std::vector<std::shared_ptr<Student>> searchByCourse(int course) const;

    bool saveToFile(const std::string& filename = "") const;
    bool loadFromFile(const std::string& filename = "");
    void setFilename(const std::string& newFilename) { filename = newFilename; }

    void clear();
};

#endif