from database import *
import time

if __name__ == "__main__":
    database = "db/marsrover.db"
    conn = create_connection(database)

    ## Test Obstacle Table
    obstacle_record_red = [["red", 1, 2, 1],
                           ["red", 3, 4, 1],
                           ["red", 5, 6, 3],
                           ["red", 7, 8, 1],
                           ["red", 9, 10, 1]]

    obstacle_record_green = [["green", 1, 2, 3],
                             ["green", 3, 4, 1],
                             ["green", 5, 6, 1],
                             ["green", 7, 8, 1],
                             ["green", 9, 10, 1]]

    obstacle_record_yellow = [["yellow", 1, 2, 1],
                              ["yellow", 3, 4, 1],
                              ["yellow", 5, 6, 2],
                              ["yellow", 7, 8, 1],
                              ["yellow", 9, 10, 1]]

    obstacle_record_blue = [["blue", 1, 2, 1],
                            ["blue", 3, 4, 1],
                            ["blue", 5, 6, 1],
                            ["blue", 7, 8, 4],
                            ["blue", 9, 10, 1]]

    obstacle_record_grey = [["grey", 1, 2, 1],
                            ["grey", 3, 4, 1],
                            ["grey", 5, 6, 1],
                            ["grey", 7, 8, 1],
                            ["grey", 9, 10, 1]]
    
    for i in [obstacle_record_red, obstacle_record_green, obstacle_record_blue, obstacle_record_grey, obstacle_record_yellow]:
        for j in i:
            create_obstacle_record(conn, j)
    
    # Test if obstacles are obtained accurately
    print("Top obstacles, rover_id=1 :")
    print(select_top_obstacle(conn, 4, 1))

    print("All obstacles, rover_id=3 :")
    print(select_all_obstacles(conn, 3))

    ## Test Position Table
    position_record_first = [[1, 2, 120, 1],
                             [2, 3, 140, 1],
                             [4, 5, 80, 1],
                             [7, 2, -12, 1],
                             [-4, -3, -90, 1]]

    position_record_second = [[4, 2, -180, 2],
                              [4, 12, 10, 2],
                              [12, -5, 180, 2],
                              [3, -12, 32, 2],
                              [-5, -15, -88, 2]]
    
    position_record_third = [[-2, 10, 20, 3],
                              [0, 0, 40, 3],
                              [10, -9, -20, 3],
                              [13, -15, -60, 3],
                              [19, -20, -89, 3]]

    for i in [position_record_first, position_record_second, position_record_third]:
        for j in i:
            create_position_record(conn, j)
    
    print("Getting all position values, rover_id=1 :")
    print(select_all_positions(conn, 1))

    print("Getting all position values, rover_id=3 :")
    print(select_all_positions(conn, 3))

    ## Test Trip Table 
    # Test if end_trip query updates end time
    id = create_trip_record(conn)
    print("Start Trip Record: ")
    print(select_all_trips(conn))

    time.sleep(1)

    end_trip(conn, id)
    print("End Trip Record: ")
    print(select_all_trips(conn))
