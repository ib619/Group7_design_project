import sqlite3
from sqlite3 import Error
from datetime import datetime

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

# Insert Data to Distance Table
def create_obstacle_record(conn, obstacle_record):
    sql_query = """INSERT INTO obstacle_record (colour, x_coord, y_coord, date)
                   VALUES(?,?,?,?)"""
    cur = conn.cursor()
    cur.execute(sql_query, obstacle_record)
    conn.commit()
    return cur.lastrowid

# Get recent 5 obstacle
def select_top_obstacle(conn):
    sql_query = """SELECT * FROM (
        SELECT colour, 
               x_coord,
               y_coord, 
               row_number() over (partition by colour order by date desc) as date_rank
        FROM obstacle_record) 
        WHERE date_rank <=5;"""
    cur = conn.cursor()
    cur.execute(sql_query)
    return cur.fetchall()

# Get all obstacles
def select_all_obstacles(conn):
    sql_query = """SELECT colour, x_coord, y_coord FROM obstacle_record GROUP BY colour"""
    cur = conn.cursor()
    cur.execute(sql_query)
    return cur.fetchall()

if __name__ == "__main__":
    database = "db/marsrover.db"
    sql_create_obstacle_history_table = """ CREATE TABLE IF NOT EXISTS obstacle_record (
                                            id integer PRIMARY KEY,
                                            colour text NOT NULL,
                                            x_coord integer NOT NULL,
                                            y_coord integer NOT NULL,
                                            date text
                                         ); """

    conn = create_connection(database)
    print("DB created")
    
    if conn is not None:
        create_table(conn, sql_create_obstacle_history_table)
    else:
        print("Error! Cannot create connection")