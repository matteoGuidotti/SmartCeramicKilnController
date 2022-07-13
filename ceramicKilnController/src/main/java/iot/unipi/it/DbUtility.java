package iot.unipi.it;

import java.sql.*;

public class DbUtility {

	 @SuppressWarnings("finally")
		private static Connection makeJDBCConnection() {
		Connection databaseConnection = null;


        String databaseIP = "localhost";
        String databasePort = "3306";
        String databaseUsername = "db_iot";
        String databasePassword = "iotproject";
        String databaseName = "db_ceramicKiln";
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");//checks if the Driver class exists (correctly available)
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return databaseConnection;
        }

        try {
            // DriverManager: The basic service for managing a set of JDBC drivers.
            databaseConnection = DriverManager.getConnection(
                    "jdbc:mysql://" + databaseIP + ":" + databasePort +
                            "/" + databaseName + "?zeroDateTimeBehavior=CONVERT_TO_NULL&serverTimezone=CET",
                    databaseUsername,
                    databasePassword);
            //The Driver Manager provides the connection specified in the parameter string
            if (databaseConnection == null) {
                System.err.println("Connection to Db failed");
            }
        } catch (SQLException e) {
        	System.err.println("MySQL Connection Failed!\n");
            e.printStackTrace();
        }finally {
            return databaseConnection;
        }
    }
	
	 
	 
	public static void insertTemperature(final int temperature) {
    	String insertQueryStatement = "INSERT INTO temperature (timestamp, temperature) VALUES (CURRENT_TIMESTAMP, ?)";
    	try (Connection smartPoolConnection = makeJDBCConnection();
        		PreparedStatement smartPoolPrepareStat = smartPoolConnection.prepareStatement(insertQueryStatement);
           ) {
    		smartPoolPrepareStat.setInt(1, temperature);        	                
        	smartPoolPrepareStat.executeUpdate();
 
    	 } catch (SQLException sqlex) {
             sqlex.printStackTrace();
         }
	}
	 
	public static void insertOxygen(final double oxygen) {
    	String insertQueryStatement = "INSERT INTO oxygen (timestamp, oxygen) VALUES (CURRENT_TIMESTAMP, ?)";
    	
    	try (Connection smartPoolConnection = makeJDBCConnection();
        		PreparedStatement smartPoolPrepareStat = smartPoolConnection.prepareStatement(insertQueryStatement);
           ) {
    		smartPoolPrepareStat.setDouble(1, oxygen);                
        	smartPoolPrepareStat.executeUpdate();
 
    	 } catch (SQLException sqlex) {
             sqlex.printStackTrace();
         }
	}
	
	//start is true if the record to be inserted indicates the detection of a fire, false if it indicates the end of the fire 
    public static void insertFireAlarm(boolean start) {
    	
    	String insertQueryStatement = "INSERT INTO fire_alarm (timestamp, alarm) VALUES (CURRENT_TIMESTAMP, ?)";
        try (Connection smartFridgeConnection = makeJDBCConnection();
        		PreparedStatement smartFridgePrepareStat = smartFridgeConnection.prepareStatement(insertQueryStatement);
           ) {
            smartFridgePrepareStat.setBoolean(1, start);
        	smartFridgePrepareStat.executeUpdate();
            
        } catch (SQLException sqlex) {
            sqlex.printStackTrace();
        }
    }
	
}
