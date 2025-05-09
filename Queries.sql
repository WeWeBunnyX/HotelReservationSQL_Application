CREATE TABLE Users (
    User_ID SERIAL PRIMARY KEY,
    Full_Name VARCHAR(100) NOT NULL,
    Email VARCHAR(100) UNIQUE NOT NULL,
    Phone_Number VARCHAR(15),
    Role VARCHAR(10) CHECK (Role IN ('Customer', 'Admin')) NOT NULL,
    Password TEXT NOT NULL -- Assuming it's already encrypted before storage
);

CREATE TABLE Rooms (
    Room_ID SERIAL PRIMARY KEY,
    Room_Type VARCHAR(50) NOT NULL,
    Price_Per_Night NUMERIC(10, 2) NOT NULL CHECK (Price_Per_Night >= 0),
    Status VARCHAR(10) NOT NULL CHECK (Status IN ('Available', 'Booked')),
    Description TEXT
);

CREATE TABLE Reservations (
    Reservation_ID SERIAL PRIMARY KEY,
    User_ID INT NOT NULL,
    Room_ID INT NOT NULL,
    Check_In_Date DATE NOT NULL,
    Check_Out_Date DATE NOT NULL,
    Total_Amount NUMERIC(10, 2) NOT NULL CHECK (Total_Amount >= 0),
    Payment_Status VARCHAR(10) NOT NULL CHECK (Payment_Status IN ('Paid', 'Pending')),
    Reservation_Status VARCHAR(10) NOT NULL CHECK (Reservation_Status IN ('Confirmed', 'Canceled')),
    FOREIGN KEY (User_ID) REFERENCES Users(User_ID) ON DELETE CASCADE,
    FOREIGN KEY (Room_ID) REFERENCES Rooms(Room_ID) ON DELETE CASCADE,
    CHECK (Check_Out_Date > Check_In_Date)
);

CREATE TABLE Feedback (
    Feedback_ID SERIAL PRIMARY KEY,
    User_ID INT NOT NULL,
    Reservation_ID INT NOT NULL,
    Rating INT NOT NULL CHECK (Rating BETWEEN 1 AND 5),
    Comments TEXT,
    Submission_Date DATE NOT NULL DEFAULT CURRENT_DATE,
    FOREIGN KEY (User_ID) REFERENCES Users(User_ID) ON DELETE CASCADE,
    FOREIGN KEY (Reservation_ID) REFERENCES Reservations(Reservation_ID) ON DELETE CASCADE
);
