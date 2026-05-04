# Group02 Course Management

## Run With Docker

Install Docker Desktop, open a terminal in this folder, then run:

```powershell
docker compose run --rm app
```

Docker will compile the C++ files and run the program inside a Linux container.
The project folder is mounted into the container, so changes to `student.csv`,
`teacher.csv`, `course.csv`, and `enrollment.csv` are saved back to this folder.

If the program is still running, use the menu option `0` to go back/logout before
closing the terminal.

## Run Without Docker

```powershell
g++ -std=c++11 main.cpp admin.cpp student.cpp teacher.cpp course.cpp tuition.cpp -o main.exe
.\main.exe
```
