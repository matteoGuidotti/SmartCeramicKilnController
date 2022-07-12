package iot.unipi.it;

import java.net.SocketException;

import org.eclipse.californium.core.CoapServer;

public class Collector extends CoapServer {

	public static void main(String[] args) {
		//TODO: richiedi i valori di ossigeno e temperatura desiderati
		float oxygen_target = 0;
		RegistrationServer rs = null;
		try {
			rs = new RegistrationServer(oxygen_target);
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		rs.start();
	}

}
