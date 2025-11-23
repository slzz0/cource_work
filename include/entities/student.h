    #ifndef STUDENT_H
    #define STUDENT_H

    #include <map>
    #include <memory>
    #include <string>

    class Student {
    private:
        std::string name;
        std::string surname;
        int course;
        int semester;
        double averageGrade;
        bool isBudget;
        double scholarship;
        int missedHours = 0;
        bool hasSocialScholarship = false;
        std::map<int, double> previousSemesterGrades;
        std::map<int, double> previousSemesterScholarships;  // История стипендий по семестрам
        int budgetSemester = -1;  // Семестр, когда студент стал бюджетником (-1 если всегда был бюджетником)

    public:
        Student(const std::string& name, const std::string& surname, int course, int semester,
                double averageGrade, bool isBudget);

        virtual ~Student() = default;

        virtual double calculateAverageGrade() const;
        virtual std::string getFullName() const;
        virtual std::string getStudentInfo() const;

        std::string getName() const { return name; }
        std::string getSurname() const { return surname; }
        int getCourse() const { return course; }
        int getSemester() const { return semester; }
        double getAverageGrade() const { return averageGrade; }
        bool getIsBudget() const { return isBudget; }
        int getMissedHours() const { return missedHours; }
        bool getHasSocialScholarship() const { return hasSocialScholarship; }
        const std::map<int, double>& getPreviousSemesterGrades() const {
            return previousSemesterGrades;
        }
        const std::map<int, double>& getPreviousSemesterScholarships() const {
            return previousSemesterScholarships;
        }
        int getBudgetSemester() const { return budgetSemester; }
        std::string getHistoryString() const;

        void setName(const std::string& name) { this->name = name; }
        void setSurname(const std::string& surname) { this->surname = surname; }
        void setCourse(int course) { this->course = course; }
        void setSemester(int semester);
        void setAverageGrade(double averageGrade) { this->averageGrade = averageGrade; }
        void setIsBudget(bool isBudget);
        void setMissedHours(int hours) { missedHours = hours; }
        void setHasSocialScholarship(bool has) { hasSocialScholarship = has; }
        void addPreviousGrade(int semester, double grade) { previousSemesterGrades[semester] = grade; }
        void clearPreviousGrades() { previousSemesterGrades.clear(); }

        void recalculateScholarship();
        double getScholarship() const { return scholarship; }
        void setScholarship(double s) { scholarship = s; }
    };

    #endif