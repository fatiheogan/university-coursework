package com.fatihemre.services;

import java.util.List;

import com.fatihemre.model.User;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class UserService {
	
	private List<User> userList;
	 
}
