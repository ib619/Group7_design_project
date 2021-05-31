import sqlite3
from sqlite3 import Error
import time

def create_connection(db_file):
    """create a database connection to a SQLite database"""
    conn = None
    try:
        conn = sqlite3.connect(db_file)
        print("DB connected")
    except Error as e:
        print(e)

    return conn

def create_table(conn, create_table_sql):
    try: 
        c = conn.cursor()
        c.execute(create_table_sql)
    except Error as e:
        print(e)

## Obstacle Table
# Insert Data to Distance Table
def create_obstacle_record(conn, obstacle_record):
    sql_query = """INSERT INTO obstacle_record (colour, x_coord, y_coord, rover_id)
                   VALUES(?,?,?,?)"""
    cur = conn.cursor()
    cur.execute(sql_query, obstacle_record)
    conn.commit()
    return cur.lastrowid

# Get recent num obstacles
def select_top_obstacle(conn, num, rover_id):
    sql_query = """SELECT * FROM (
                        SELECT 
                            colour, 
                            x_coord, 
                            y_coord, 
                            row_number() over (PARTITION BY colour ORDER BY date DESC) as date_rank
                        FROM obstacle_record
                        WHERE rover_id=?) 
                    WHERE date_rank <=?;"""
    cur = conn.cursor()
    cur.execute(sql_query, [rover_id, num])
    return cur.fetchall()

# Get all obstacles
def select_all_obstacles(conn, rover_id):
    sql_query = """SELECT colour, x_coord, y_coord FROM obstacle_record 
                    WHERE rover_id=? 
                    GROUP BY colour;"""
    cur = conn.cursor()
    cur.execute(sql_query, [rover_id])
    return cur.fetchall()

## Position Table
# Insert Data into Position Table
def create_position_record(conn, position_record):
    sql_query = """INSERT INTO position_record (x_coord, y_coord, heading, rover_id)
                   VALUES(?,?,?,?)"""
    cur = conn.cursor()
    cur.execute(sql_query, position_record)
    conn.commit()
    return cur.lastrowid

# Get all past positions
def select_all_positions(conn, rover_id):
    # ascending means least recent data is first
    sql_query = """SELECT x_coord, y_coord FROM position_record 
                    WHERE rover_id=?                   
                    ORDER BY date ASC;"""
    cur = conn.cursor()
    cur.execute(sql_query, [rover_id])
    return cur.fetchall()

# Trip table
def create_trip_record(conn):
    sql_query = """INSERT INTO trip_record DEFAULT VALUES;"""
    cur = conn.cursor()
    cur.execute(sql_query)
    conn.commit()
    return cur.lastrowid # returns the current rover_id

def end_trip(conn, rover_id):
    sql_query = """UPDATE trip_record SET end=datetime('now') WHERE id=?;"""
    cur = conn.cursor()
    cur.execute(sql_query, [rover_id])
    conn.commit()
    return cur.lastrowid # returns the current rover_id

def select_all_trips(conn):
    sql_query = """SELECT * FROM trip_record;"""
    cur = conn.cursor()
    cur.execute(sql_query)
    return cur.fetchall() # returns the current rover_id

if __name__ == "__main__":
    database = "db/marsrover.db"
    sql_create_obstacle_history_table = """ CREATE TABLE IF NOT EXISTS obstacle_record (
                                            id integer PRIMARY KEY,
                                            colour text NOT NULL,
                                            x_coord integer NOT NULL,
                                            y_coord integer NOT NULL,
                                            rover_id integer NOT NULL, 
                                            date datetime DEFAULT CURRENT_TIMESTAMP,
                                            FOREIGN KEY (rover_id) REFERENCES trip_record (id)
                                         ); """

    sql_create_position_history_table = """ CREATE TABLE IF NOT EXISTS position_record (
                                            id integer PRIMARY KEY,
                                            x_coord integer NOT NULL,
                                            y_coord integer NOT NULL,
                                            heading integeer NOT NULL,
                                            rover_id integer NOT NULL, 
                                            date datetime DEFAULT CURRENT_TIMESTAMP,
                                            FOREIGN KEY (rover_id) REFERENCES trip_record (id)
                                        ); """

    sql_create_trip_history_table = """ CREATE TABLE IF NOT EXISTS trip_record (
                                        id integer PRIMARY KEY,
                                        start datetime DEFAULT CURRENT_TIMESTAMP,
                                        end datetime DEFAULT CURRENT_TIMESTAMP
                                    );"""

    
    conn = create_connection(database)
    print("DB created")
    
    if conn is not None:
        create_table(conn, sql_create_trip_history_table)
        create_table(conn, sql_create_obstacle_history_table)
        create_table(conn, sql_create_position_history_table)
    else:
        print("Error! Cannot create connection")