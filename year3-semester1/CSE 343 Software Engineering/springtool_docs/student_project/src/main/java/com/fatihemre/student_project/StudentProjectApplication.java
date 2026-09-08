package com.fatihemre.student_project;

import com.fatihemre.student_project.entity.Student;
import com.fatihemre.student_project.repository.StudentRepository;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;

@SpringBootApplication
@EnableJpaRepositories(basePackages = "com.fatihemre.student_project.repository")
public class StudentProjectApplication {

    public static void main(String[] args) {
        ConfigurableApplicationContext context = SpringApplication.run(StudentProjectApplication.class, args);

        StudentRepository repository = context.getBean(StudentRepository.class);

        Student student = new Student();
        student.setName("Mıstık");
        student.setGrade(85.0);
        repository.save(student);
        System.out.println("Kaydedildi! ID: " + student.getId());

        Student student2 = new Student();
        student2.setName("Enes");
        student2.setGrade(92.5);
        repository.save(student2);
        System.out.println("Kaydedildi! ID: " + student2.getId());
    }
}
