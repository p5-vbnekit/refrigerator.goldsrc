include("FetchContent")

FetchContent_Declare(
    "p5-lambda"
    GIT_REPOSITORY "https://github.com/p5-vbnekit/lambda.goldsrc"
    GIT_TAG "55df29fc862d930f1de84aed82a9e68e6c4440bf"
)

FetchContent_MakeAvailable("p5-lambda")
