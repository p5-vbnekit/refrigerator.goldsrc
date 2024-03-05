include("FetchContent")

FetchContent_Declare(
    "p5-lambda"
    GIT_REPOSITORY "https://github.com/p5-vbnekit/lambda.goldsrc"
    GIT_TAG "4effd7aa37f6d7a4ae554249736d469e68be22b2"
)

FetchContent_MakeAvailable("p5-lambda")
