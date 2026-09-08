package com.fatihemre.config;

import java.util.ArrayList;
import java.util.List;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import com.fatihemre.model.User;
import com.fatihemre.services.UserService;

@Configuration
public class AppConfig {

	@Bean
	public UserService userService() {

		UserService userService = new UserService();

		List<User> userList = new ArrayList<>();
		userList.add(new User("Fatihemre"));
		userList.add(new User("OGAN"));

		userService.setUserList(userList);

		return userService; 
		
	} 

}
