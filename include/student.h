#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <memory>

/**
 * @brief Base class for Student with virtual functions
 */
class Student {
protected:
    std::string name;
    std::string surname;
    int course;
    int semester;
    double averageGrade;
    bool isBudget;

public:
    Student(const std::string& name, const std::string& surname,
            int course, int semester, double averageGrade, bool isBudget);
    
    virtual ~Student() = default;

    // Virtual functions
    virtual double calculateAverageGrade() const;
    virtual std::string getFullName() const;
    virtual std::string getStudentInfo() const;
    
    // Getters
    std::string getName() const { return name; }
    std::string getSurname() const { return surname; }
    int getCourse() const { return course; }
    int getSemester() const { return semester; }
    double getAverageGrade() const { return averageGrade; }
    bool getIsBudget() const { return isBudget; }
    
    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setSurname(const std::string& surname) { this->surname = surname; }
    void setCourse(int course) { this->course = course; }
    void setSemester(int semester) { this->semester = semester; }
    void setAverageGrade(double averageGrade) { this->averageGrade = averageGrade; }
    void setIsBudget(bool isBudget) { this->isBudget = isBudget; }
};

#endif // STUDENT_H

