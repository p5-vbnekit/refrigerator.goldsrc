include("FetchContent")

FetchContent_Declare(
    "p5-lambda"
    GIT_REPOSITORY "https://github.com/p5-vbnekit/lambda.goldsrc"
    GIT_TAG "3b08d2615b6f73eb17c74684724fba09d430a9b3"
)

FetchContent_MakeAvailable("p5-lambda")
