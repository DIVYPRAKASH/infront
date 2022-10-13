1. Build docker image
```
docker build -t infront:latest -f Dockerfile.infront .
```

2. Use the above docker image to build
```
mkdir Release && "$_"
../runUsingDocker cmake ../src/
../runUsingDocker make -j4
../runUsingDocker ./bin/infront <FILEPATH>
```

3. There should be a output.txt file generated in the Release folder which should look like this: (This corresponds to LEVEL 3 of the task)
```
"FileX,FileY"
	"DirA"
    "DirB"
	"DirB/DirBB"
	"DirC/DirCC"
"FileYY"
	"DirB/DirBB"
	"DirC/DirCC"
"FileZ"
	"DirA"
	"DirB"
	"DirC"
```