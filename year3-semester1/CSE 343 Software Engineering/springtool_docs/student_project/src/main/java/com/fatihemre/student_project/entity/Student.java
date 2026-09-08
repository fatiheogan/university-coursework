package com.fatihemre.student_project.entity;

import jakarta.persistence.*;

@Entity
public class Student {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private String name;
    private Double grade;

    // Getter ve Setter

    public Long getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public Double getGrade() {
        return grade;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public void setName(String name) {
        this.name = name;
    }

    public void setGrade(Double grade) {
        this.grade = grade;
    }
}
