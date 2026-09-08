package com.fatihemre.services;

import org.springframework.context.annotation.AnnotationConfigApplicationContext;

import com.fatihemre.config.AppConfig;

public class LoginService {

		public void login() {
			//User Listesi ihtiyac
			AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(AppConfig.class);
			UserService UserService1= context.getBean(UserService.class);
		}
}
