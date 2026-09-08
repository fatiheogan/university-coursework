package com.fatihemre.main;

import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import com.fatihemre.model.User;

import com.fatihemre.config.AppConfig;
import com.fatihemre.services.LoginService;
import com.fatihemre.services.UserService;

public class MainClass {

	public static void main(String[] args) {
		
		
		AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(AppConfig.class);
		UserService UserService1= context.getBean(UserService.class);
		
		for (User user  : UserService1.getUserList()) {
			System.out.println(user);
		}
		
		LoginService loginService = new LoginService();
		loginService.login();
	}

}
