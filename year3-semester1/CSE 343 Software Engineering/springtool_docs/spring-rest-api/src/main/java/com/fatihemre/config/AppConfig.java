package com.fatihemre.config;

import java.util.ArrayList;
import java.util.List;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import com.fatihemre.model.Employee;

@Configuration
public class AppConfig {

	@Bean
	public List<Employee> employeeList() {

		List<Employee> employeeList = new ArrayList<>();
		employeeList.add(new Employee("1", "Fatih Emre", "Oğan"));
		employeeList.add(new Employee("2", "Enes", "Ersoy"));
		employeeList.add(new Employee("3", "Selim", "Kasım"));
		employeeList.add(new Employee("4", "Alper", "Kahraman"));
		employeeList.add(new Employee("5", "Bilal", "Tümer"));
		employeeList.add(new Employee("6", "Fatih Emre", "Soğanr"));
		employeeList.add(new Employee("5", "Enes", "Malı"));

		return employeeList;

	}

}
