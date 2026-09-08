package com.fatihemre.student_project.controller;

import java.util.Map;
import com.fatihemre.student_project.entity.Student;
import com.fatihemre.student_project.repository.StudentRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/students")
public class StudentController {

    @Autowired
    private StudentRepository studentRepository;

    @GetMapping
    public List<Student> getAllStudents() {
        return studentRepository.findAll();
    }

    @PostMapping
    public Student addStudent(@RequestBody Student student) {
        return studentRepository.save(student);
    }

    @GetMapping("/{id}")
    public Student getStudentById(@PathVariable Long id) {
        return studentRepository.findById(id).orElse(null);
    }

    @PutMapping("/{id}")
    public Student updateStudent(@PathVariable Long id, @RequestBody Student updatedStudent) {
        return studentRepository.findById(id)
                .map(student -> {
                    student.setName(updatedStudent.getName());
                    student.setGrade(updatedStudent.getGrade());
                    return studentRepository.save(student);
                })
                .orElse(null);
    }

    @DeleteMapping("/{id}")
    public String deleteStudent(@PathVariable Long id) {
        if (studentRepository.existsById(id)) {
            studentRepository.deleteById(id);
            return "Silindi";
        } else {
            return "Öğrenci bulunamadı";
        }
    }
    
    @PatchMapping("/{id}")
    public Student partialUpdateStudent(@PathVariable Long id, @RequestBody Map<String, Object> updates) {
        return studentRepository.findById(id)
                .map(student -> {
                    if (updates.containsKey("name")) {
                        student.setName((String) updates.get("name"));
                    }
                    if (updates.containsKey("grade")) {
                        Object gradeObj = updates.get("grade");
                        if (gradeObj instanceof Number) {
                            student.setGrade(((Number) gradeObj).doubleValue());
                        } else {
                            try {
                                student.setGrade(Double.valueOf(gradeObj.toString()));
                            } catch (NumberFormatException e) {
                                // geçersiz not verisi varsa atla
                            }
                        }
                    }
                    return studentRepository.save(student);
                })
                .orElse(null);
    }

    // OPTIONS /students için
    @RequestMapping(method = RequestMethod.OPTIONS)
    public ResponseEntity<Void> optionsStudents() {
        return ResponseEntity.ok()
                .allow(HttpMethod.GET, HttpMethod.POST, HttpMethod.PUT, HttpMethod.DELETE, HttpMethod.PATCH, HttpMethod.OPTIONS)
                .build();
    }

    // OPTIONS /students/{id} için
    @RequestMapping(value = "/{id}", method = RequestMethod.OPTIONS)
    public ResponseEntity<Void> optionsStudentById(@PathVariable Long id) {
        return ResponseEntity.ok()
                .allow(HttpMethod.GET, HttpMethod.PUT, HttpMethod.DELETE, HttpMethod.PATCH, HttpMethod.OPTIONS)
                .build();
    }
}
