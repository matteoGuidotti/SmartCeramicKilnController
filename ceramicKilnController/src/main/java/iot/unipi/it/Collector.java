package iot.unipi.it;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.SocketException;

import javax.swing.text.DefaultStyledDocument.ElementSpec;

import org.eclipse.californium.core.CoapServer;
import org.eclipse.paho.client.mqttv3.MqttException;

public class Collector extends CoapServer {
	private static final String[] requestsArray = {"Insert the target temperature: ", "Insert the acceptable range for the temperature: ",
													"Insert the target oxygen values: ", "Insert the acceptable range for the oxygen value: "}; 

	public static void main(String[] args) throws MqttException, IOException {
		//TODO: richiedi i valori di ossigeno e temperatura desiderati
		double oxygen_target = 0;
		double acceptableOxygenRange = 0;
		int targetTemp = 0;
		int acceptableTempRange = 0;
		RegistrationServer rs = null;
		MqttClientCollector mqttClient = null;

		//to read inputs from the console
		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));

		String command = "";
		String[] tokens;
		for(int i = 0; i < 4; i++){
			System.out.print(requestsArray[i]);
			command = reader.readLine();
			tokens = command.split(" ");
			if(i == 0)
				targetTemp = Integer.parseInt(tokens[0]);
			else if(i == 1)
				acceptableTempRange = Integer.parseInt(tokens[0]);
			else if(i == 2)
				oxygen_target = Double.parseDouble(tokens[0]);
			else if(i == 3)
				acceptableOxygenRange = Double.parseDouble(tokens[0]);
			System.out.println();
		}

		mqttClient = new MqttClientCollector(targetTemp, acceptableTempRange);

		rs = new RegistrationServer(oxygen_target, acceptableOxygenRange);
		rs.start();

		printCommands();

		while(true){
			command = reader.readLine();
			tokens = command.split(" ");
			String insertedCommand = tokens[0];
			if(insertedCommand.equals("!exit")){
				System.out.println("Bye bye");
				System.exit(1);
			}
			else if(insertedCommand.equals("!commands")){
				printCommands();
			}
			else if(insertedCommand.equals("!changeTargetTemp")){
				if(tokens.length < 2){
					System.out.println("Wrong command format");
					printCommandError();
					continue;
				}
				mqttClient.changeTargetTemp(Integer.parseInt(tokens[1]));
				System.out.println("Target temperature changed succesfully!");
			}
			else if(insertedCommand.equals("!changeAccTemp")){
				if(tokens.length < 2){
					System.out.println("Wrong command format");
					printCommandError();
					continue;
				}
				mqttClient.changeAccTemp(Integer.parseInt(tokens[1]));
				System.out.println("Acceptable range for the temeprature changed succesfully");
			}
			else if(insertedCommand.equals("!changeTargetOxygen")){
				if(tokens.length < 2){
					System.out.println("Wrong command format");
					printCommandError();
					continue;
				}
				rs.changeTargetOxygen(Double.parseDouble(tokens[1]));
				System.out.println("Target oxygen value changed succesfully");
			}
			else if(insertedCommand.equals("!changeAccOxygen")){
				if(tokens.length < 2){
					System.out.println("Wrong command format");
					printCommandError();
					continue;
				}
				rs.changeAccOxygen(Double.parseDouble(tokens[1]));
				System.out.println("Acceptable range for the oxygen value changed succesfully");
			}
			else if(insertedCommand.equals("!stopAlarm")){
				
			}
			else{
				printCommandError();
			}
		}
	}

	public static void printCommands(){
		System.out.println("\nCommand list:");
		System.out.println("!exit: exit the program");
		System.out.println("!commands: list possible commands");					
		System.out.println("!changeTargetTemp [new_value]: set new target temperature value");
		System.out.println("!changeAccTemp [new_value]: set new acceptable range for the temperature");
		System.out.println("!changeTargetOxygen [new_value]: set new target oxygen value");
		System.out.println("!changeAccOxygen [new_value]: set new acceptable range for the oxygen");
		System.out.println("!stopAlarm: stop the fire alarm");
		System.out.println("\n");
	}

	public static void printCommandError(){
		System.out.println("The command cannot be recognised");
	}
}
