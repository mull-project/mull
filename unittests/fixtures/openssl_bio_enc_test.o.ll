; ModuleID = '/Users/alexdenisov/Projects/OpenSSL/openssl/test/bio_enc_test.o'
source_filename = "test/bio_enc_test.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

%struct.evp_cipher_st = type opaque
%struct.bio_st = type opaque
%struct.bio_method_st = type opaque

@.str = private unnamed_addr constant [25 x i8] c"test_bio_enc_aes_128_cbc\00", align 1
@.str.1 = private unnamed_addr constant [25 x i8] c"test_bio_enc_aes_128_ctr\00", align 1
@.str.2 = private unnamed_addr constant [25 x i8] c"test_bio_enc_aes_256_cfb\00", align 1
@.str.3 = private unnamed_addr constant [25 x i8] c"test_bio_enc_aes_256_ofb\00", align 1
@.str.4 = private unnamed_addr constant [22 x i8] c"test_bio_enc_chacha20\00", align 1
@.str.5 = private unnamed_addr constant [31 x i8] c"test_bio_enc_chacha20_poly1305\00", align 1
@KEY = internal constant [32 x i8] c"QP\D1w/P\83JP>\06\9A\97?\BD|\E6\1CC+r\0B\19\D1\8E\C8\D8K\DCc\15\1B", align 16
@IV = internal constant [32 x i8] c"\01\02\03\04\05\06\07\08\01\02\03\04\05\06\07\08\01\02\03\04\05\06\07\08\01\02\03\04\05\06\07\08", align 16
@do_bio_cipher.inp = internal global [1056 x i8] zeroinitializer, align 16
@.str.6 = private unnamed_addr constant [20 x i8] c"test/bio_enc_test.c\00", align 1
@.str.7 = private unnamed_addr constant [27 x i8] c"RAND_bytes(inp, DATA_SIZE)\00", align 1
@.str.8 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@.str.9 = private unnamed_addr constant [44 x i8] c"BIO_set_cipher(b, cipher, key, iv, ENCRYPT)\00", align 1
@.str.10 = private unnamed_addr constant [36 x i8] c"Split encrypt failed @ operation %d\00", align 1
@.str.11 = private unnamed_addr constant [7 x i8] c"out[i]\00", align 1
@.str.12 = private unnamed_addr constant [23 x i8] c"(unsigned char)~ref[i]\00", align 1
@.str.13 = private unnamed_addr constant [45 x i8] c"Encrypt overstep check failed @ operation %d\00", align 1
@.str.14 = private unnamed_addr constant [4 x i8] c"out\00", align 1
@.str.15 = private unnamed_addr constant [4 x i8] c"ref\00", align 1
@.str.16 = private unnamed_addr constant [38 x i8] c"Encrypt compare failed @ operation %d\00", align 1
@.str.17 = private unnamed_addr constant [42 x i8] c"Small chunk encrypt failed @ operation %d\00", align 1
@.str.18 = private unnamed_addr constant [50 x i8] c"Small chunk encrypt compare failed @ operation %d\00", align 1
@.str.19 = private unnamed_addr constant [44 x i8] c"BIO_set_cipher(b, cipher, key, iv, DECRYPT)\00", align 1
@.str.20 = private unnamed_addr constant [4 x i8] c"inp\00", align 1
@.str.21 = private unnamed_addr constant [36 x i8] c"Split decrypt failed @ operation %d\00", align 1
@.str.22 = private unnamed_addr constant [45 x i8] c"Decrypt overstep check failed @ operation %d\00", align 1
@.str.23 = private unnamed_addr constant [38 x i8] c"Decrypt compare failed @ operation %d\00", align 1
@.str.24 = private unnamed_addr constant [42 x i8] c"Small chunk decrypt failed @ operation %d\00", align 1
@.str.25 = private unnamed_addr constant [50 x i8] c"Small chunk decrypt compare failed @ operation %d\00", align 1

; Function Attrs: nounwind ssp uwtable
define void @setup_single_tests() #0 !dbg !31 {
  %1 = alloca i8*, align 8
  call void @llvm.dbg.declare(metadata i8** %1, metadata !34, metadata !37), !dbg !38
  %2 = call i8* @test_get_argument(i64 0), !dbg !39
  store i8* %2, i8** %1, align 8, !dbg !38
  %3 = load i8*, i8** %1, align 8, !dbg !40
  %4 = call i32 @strcmp(i8* %3, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0)), !dbg !40
  %5 = icmp eq i32 %4, 0, !dbg !40
  br i1 %5, label %6, label %7, !dbg !42

; <label>:6:                                      ; preds = %0
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_128_cbc, i32 2, i32 1), !dbg !43
  br label %7, !dbg !43

; <label>:7:                                      ; preds = %6, %0
  %8 = load i8*, i8** %1, align 8, !dbg !45
  %9 = call i32 @strcmp(i8* %8, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.1, i32 0, i32 0)), !dbg !45
  %10 = icmp eq i32 %9, 0, !dbg !45
  br i1 %10, label %11, label %12, !dbg !47

; <label>:11:                                     ; preds = %7
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.1, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_128_ctr, i32 2, i32 1), !dbg !48
  br label %12, !dbg !48

; <label>:12:                                     ; preds = %11, %7
  %13 = load i8*, i8** %1, align 8, !dbg !50
  %14 = call i32 @strcmp(i8* %13, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.2, i32 0, i32 0)), !dbg !50
  %15 = icmp eq i32 %14, 0, !dbg !50
  br i1 %15, label %16, label %17, !dbg !52

; <label>:16:                                     ; preds = %12
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.2, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_256_cfb, i32 2, i32 1), !dbg !53
  br label %17, !dbg !53

; <label>:17:                                     ; preds = %16, %12
  %18 = load i8*, i8** %1, align 8, !dbg !55
  %19 = call i32 @strcmp(i8* %18, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.3, i32 0, i32 0)), !dbg !55
  %20 = icmp eq i32 %19, 0, !dbg !55
  br i1 %20, label %21, label %22, !dbg !57

; <label>:21:                                     ; preds = %17
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.3, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_256_ofb, i32 2, i32 1), !dbg !58
  br label %22, !dbg !58

; <label>:22:                                     ; preds = %21, %17
  %23 = load i8*, i8** %1, align 8, !dbg !60
  %24 = call i32 @strcmp(i8* %23, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.4, i32 0, i32 0)), !dbg !60
  %25 = icmp eq i32 %24, 0, !dbg !60
  br i1 %25, label %26, label %27, !dbg !62

; <label>:26:                                     ; preds = %22
  call void @add_all_tests(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.4, i32 0, i32 0), i32 (i32)* @test_bio_enc_chacha20, i32 2, i32 1), !dbg !63
  br label %27, !dbg !63

; <label>:27:                                     ; preds = %26, %22
  %28 = load i8*, i8** %1, align 8, !dbg !65
  %29 = call i32 @strcmp(i8* %28, i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.5, i32 0, i32 0)), !dbg !65
  %30 = icmp eq i32 %29, 0, !dbg !65
  br i1 %30, label %31, label %32, !dbg !67

; <label>:31:                                     ; preds = %27
  call void @add_all_tests(i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.5, i32 0, i32 0), i32 (i32)* @test_bio_enc_chacha20_poly1305, i32 2, i32 1), !dbg !68
  br label %32, !dbg !68

; <label>:32:                                     ; preds = %31, %27
  ret void, !dbg !70
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i8* @test_get_argument(i64) #2

declare i32 @strcmp(i8*, i8*) #2

declare void @add_all_tests(i8*, i32 (i32)*, i32, i32) #2

; Function Attrs: nounwind ssp uwtable
define internal i32 @test_bio_enc_aes_128_cbc(i32) #0 !dbg !71 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !74, metadata !37), !dbg !75
  %3 = call %struct.evp_cipher_st* @EVP_aes_128_cbc(), !dbg !76
  %4 = load i32, i32* %2, align 4, !dbg !77
  %5 = call i32 @do_test_bio_cipher(%struct.evp_cipher_st* %3, i32 %4), !dbg !78
  ret i32 %5, !dbg !79
}

; Function Attrs: nounwind ssp uwtable
define internal i32 @test_bio_enc_aes_128_ctr(i32) #0 !dbg !80 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !81, metadata !37), !dbg !82
  %3 = call %struct.evp_cipher_st* @EVP_aes_128_ctr(), !dbg !83
  %4 = load i32, i32* %2, align 4, !dbg !84
  %5 = call i32 @do_test_bio_cipher(%struct.evp_cipher_st* %3, i32 %4), !dbg !85
  ret i32 %5, !dbg !86
}

; Function Attrs: nounwind ssp uwtable
define internal i32 @test_bio_enc_aes_256_cfb(i32) #0 !dbg !87 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !88, metadata !37), !dbg !89
  %3 = call %struct.evp_cipher_st* @EVP_aes_256_cfb128(), !dbg !90
  %4 = load i32, i32* %2, align 4, !dbg !91
  %5 = call i32 @do_test_bio_cipher(%struct.evp_cipher_st* %3, i32 %4), !dbg !92
  ret i32 %5, !dbg !93
}

; Function Attrs: nounwind ssp uwtable
define internal i32 @test_bio_enc_aes_256_ofb(i32) #0 !dbg !94 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !95, metadata !37), !dbg !96
  %3 = call %struct.evp_cipher_st* @EVP_aes_256_ofb(), !dbg !97
  %4 = load i32, i32* %2, align 4, !dbg !98
  %5 = call i32 @do_test_bio_cipher(%struct.evp_cipher_st* %3, i32 %4), !dbg !99
  ret i32 %5, !dbg !100
}

; Function Attrs: nounwind ssp uwtable
define internal i32 @test_bio_enc_chacha20(i32) #0 !dbg !101 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !102, metadata !37), !dbg !103
  %3 = call %struct.evp_cipher_st* @EVP_chacha20(), !dbg !104
  %4 = load i32, i32* %2, align 4, !dbg !105
  %5 = call i32 @do_test_bio_cipher(%struct.evp_cipher_st* %3, i32 %4), !dbg !106
  ret i32 %5, !dbg !107
}

; Function Attrs: nounwind ssp uwtable
define internal i32 @test_bio_enc_chacha20_poly1305(i32) #0 !dbg !108 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !109, metadata !37), !dbg !110
  %3 = call %struct.evp_cipher_st* @EVP_chacha20_poly1305(), !dbg !111
  %4 = load i32, i32* %2, align 4, !dbg !112
  %5 = call i32 @do_test_bio_cipher(%struct.evp_cipher_st* %3, i32 %4), !dbg !113
  ret i32 %5, !dbg !114
}

; Function Attrs: nounwind ssp uwtable
define i32 @setup_tests() #0 !dbg !115 {
  %1 = alloca i32, align 4
  %2 = call i64 @test_get_argument_count(), !dbg !118
  %3 = icmp ugt i64 %2, 0, !dbg !120
  br i1 %3, label %4, label %5, !dbg !121

; <label>:4:                                      ; preds = %0
  call void @setup_single_tests(), !dbg !122
  store i32 1, i32* %1, align 4, !dbg !124
  br label %6, !dbg !124

; <label>:5:                                      ; preds = %0
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_128_cbc, i32 2, i32 1), !dbg !125
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.1, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_128_ctr, i32 2, i32 1), !dbg !126
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.2, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_256_cfb, i32 2, i32 1), !dbg !127
  call void @add_all_tests(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.3, i32 0, i32 0), i32 (i32)* @test_bio_enc_aes_256_ofb, i32 2, i32 1), !dbg !128
  call void @add_all_tests(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.4, i32 0, i32 0), i32 (i32)* @test_bio_enc_chacha20, i32 2, i32 1), !dbg !129
  call void @add_all_tests(i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.5, i32 0, i32 0), i32 (i32)* @test_bio_enc_chacha20_poly1305, i32 2, i32 1), !dbg !130
  store i32 1, i32* %1, align 4, !dbg !131
  br label %6, !dbg !131

; <label>:6:                                      ; preds = %5, %4
  %7 = load i32, i32* %1, align 4, !dbg !132
  ret i32 %7, !dbg !132
}

declare i64 @test_get_argument_count() #2

; Function Attrs: nounwind ssp uwtable
define internal i32 @do_test_bio_cipher(%struct.evp_cipher_st*, i32) #0 !dbg !133 {
  %3 = alloca i32, align 4
  %4 = alloca %struct.evp_cipher_st*, align 8
  %5 = alloca i32, align 4
  store %struct.evp_cipher_st* %0, %struct.evp_cipher_st** %4, align 8
  call void @llvm.dbg.declare(metadata %struct.evp_cipher_st** %4, metadata !136, metadata !37), !dbg !137
  store i32 %1, i32* %5, align 4
  call void @llvm.dbg.declare(metadata i32* %5, metadata !138, metadata !37), !dbg !139
  %6 = load i32, i32* %5, align 4, !dbg !140
  switch i32 %6, label %13 [
    i32 0, label %7
    i32 1, label %10
  ], !dbg !141

; <label>:7:                                      ; preds = %2
  %8 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %4, align 8, !dbg !142
  %9 = call i32 @do_bio_cipher(%struct.evp_cipher_st* %8, i8* getelementptr inbounds ([32 x i8], [32 x i8]* @KEY, i32 0, i32 0), i8* null), !dbg !144
  store i32 %9, i32* %3, align 4, !dbg !145
  br label %14, !dbg !145

; <label>:10:                                     ; preds = %2
  %11 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %4, align 8, !dbg !146
  %12 = call i32 @do_bio_cipher(%struct.evp_cipher_st* %11, i8* getelementptr inbounds ([32 x i8], [32 x i8]* @KEY, i32 0, i32 0), i8* getelementptr inbounds ([32 x i8], [32 x i8]* @IV, i32 0, i32 0)), !dbg !147
  store i32 %12, i32* %3, align 4, !dbg !148
  br label %14, !dbg !148

; <label>:13:                                     ; preds = %2
  store i32 0, i32* %3, align 4, !dbg !149
  br label %14, !dbg !149

; <label>:14:                                     ; preds = %13, %10, %7
  %15 = load i32, i32* %3, align 4, !dbg !150
  ret i32 %15, !dbg !150
}

declare %struct.evp_cipher_st* @EVP_aes_128_cbc() #2

; Function Attrs: nounwind ssp uwtable
define internal i32 @do_bio_cipher(%struct.evp_cipher_st*, i8*, i8*) #0 !dbg !9 {
  %4 = alloca i32, align 4
  %5 = alloca %struct.evp_cipher_st*, align 8
  %6 = alloca i8*, align 8
  %7 = alloca i8*, align 8
  %8 = alloca %struct.bio_st*, align 8
  %9 = alloca [1056 x i8], align 16
  %10 = alloca [1056 x i8], align 16
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca i32, align 4
  store %struct.evp_cipher_st* %0, %struct.evp_cipher_st** %5, align 8
  call void @llvm.dbg.declare(metadata %struct.evp_cipher_st** %5, metadata !151, metadata !37), !dbg !152
  store i8* %1, i8** %6, align 8
  call void @llvm.dbg.declare(metadata i8** %6, metadata !153, metadata !37), !dbg !154
  store i8* %2, i8** %7, align 8
  call void @llvm.dbg.declare(metadata i8** %7, metadata !155, metadata !37), !dbg !156
  call void @llvm.dbg.declare(metadata %struct.bio_st** %8, metadata !157, metadata !37), !dbg !161
  call void @llvm.dbg.declare(metadata [1056 x i8]* %9, metadata !162, metadata !37), !dbg !163
  call void @llvm.dbg.declare(metadata [1056 x i8]* %10, metadata !164, metadata !37), !dbg !165
  call void @llvm.dbg.declare(metadata i32* %11, metadata !166, metadata !37), !dbg !167
  call void @llvm.dbg.declare(metadata i32* %12, metadata !168, metadata !37), !dbg !169
  call void @llvm.dbg.declare(metadata i32* %13, metadata !170, metadata !37), !dbg !171
  %16 = call i32 @RAND_bytes(i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i32 1024), !dbg !172
  %17 = call i32 @test_int_gt(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 47, i8* getelementptr inbounds ([27 x i8], [27 x i8]* @.str.7, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.8, i32 0, i32 0), i32 %16, i32 0), !dbg !172
  %18 = icmp ne i32 %17, 0, !dbg !172
  br i1 %18, label %20, label %19, !dbg !174

; <label>:19:                                     ; preds = %3
  store i32 0, i32* %4, align 4, !dbg !175
  br label %342, !dbg !175

; <label>:20:                                     ; preds = %3
  %21 = call %struct.bio_method_st* @BIO_f_cipher(), !dbg !176
  %22 = call %struct.bio_st* @BIO_new(%struct.bio_method_st* %21), !dbg !177
  store %struct.bio_st* %22, %struct.bio_st** %8, align 8, !dbg !178
  %23 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !179
  %24 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %5, align 8, !dbg !179
  %25 = load i8*, i8** %6, align 8, !dbg !179
  %26 = load i8*, i8** %7, align 8, !dbg !179
  %27 = call i32 @BIO_set_cipher(%struct.bio_st* %23, %struct.evp_cipher_st* %24, i8* %25, i8* %26, i32 1), !dbg !179
  %28 = icmp ne i32 %27, 0, !dbg !179
  %29 = zext i1 %28 to i32, !dbg !179
  %30 = call i32 @test_true(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 54, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.9, i32 0, i32 0), i32 %29), !dbg !179
  %31 = icmp ne i32 %30, 0, !dbg !179
  br i1 %31, label %33, label %32, !dbg !181

; <label>:32:                                     ; preds = %20
  store i32 0, i32* %4, align 4, !dbg !182
  br label %342, !dbg !182

; <label>:33:                                     ; preds = %20
  %34 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !183
  %35 = call %struct.bio_st* @BIO_new_mem_buf(i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i32 1024), !dbg !184
  %36 = call %struct.bio_st* @BIO_push(%struct.bio_st* %34, %struct.bio_st* %35), !dbg !185
  %37 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !186
  %38 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i32 0, i32 0, !dbg !187
  %39 = call i32 @BIO_read(%struct.bio_st* %37, i8* %38, i32 1056), !dbg !188
  store i32 %39, i32* %12, align 4, !dbg !189
  %40 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !190
  call void @BIO_free_all(%struct.bio_st* %40), !dbg !191
  store i32 1, i32* %11, align 4, !dbg !192
  br label %41, !dbg !194

; <label>:41:                                     ; preds = %118, %33
  %42 = load i32, i32* %11, align 4, !dbg !195
  %43 = load i32, i32* %12, align 4, !dbg !197
  %44 = icmp slt i32 %42, %43, !dbg !198
  br i1 %44, label %45, label %121, !dbg !199

; <label>:45:                                     ; preds = %41
  %46 = call %struct.bio_method_st* @BIO_f_cipher(), !dbg !200
  %47 = call %struct.bio_st* @BIO_new(%struct.bio_method_st* %46), !dbg !202
  store %struct.bio_st* %47, %struct.bio_st** %8, align 8, !dbg !203
  %48 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !204
  %49 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %5, align 8, !dbg !204
  %50 = load i8*, i8** %6, align 8, !dbg !204
  %51 = load i8*, i8** %7, align 8, !dbg !204
  %52 = call i32 @BIO_set_cipher(%struct.bio_st* %48, %struct.evp_cipher_st* %49, i8* %50, i8* %51, i32 1), !dbg !204
  %53 = icmp ne i32 %52, 0, !dbg !204
  %54 = zext i1 %53 to i32, !dbg !204
  %55 = call i32 @test_true(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 63, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.9, i32 0, i32 0), i32 %54), !dbg !204
  %56 = icmp ne i32 %55, 0, !dbg !204
  br i1 %56, label %59, label %57, !dbg !206

; <label>:57:                                     ; preds = %45
  %58 = load i32, i32* %11, align 4, !dbg !207
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 64, i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.10, i32 0, i32 0), i32 %58), !dbg !207
  store i32 0, i32* %4, align 4, !dbg !209
  br label %342, !dbg !209

; <label>:59:                                     ; preds = %45
  %60 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !210
  %61 = call %struct.bio_st* @BIO_new_mem_buf(i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i32 1024), !dbg !211
  %62 = call %struct.bio_st* @BIO_push(%struct.bio_st* %60, %struct.bio_st* %61), !dbg !212
  %63 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !213
  call void @llvm.memset.p0i8.i64(i8* %63, i8 0, i64 1056, i32 16, i1 false), !dbg !213
  %64 = load i32, i32* %11, align 4, !dbg !214
  %65 = sext i32 %64 to i64, !dbg !215
  %66 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i64 0, i64 %65, !dbg !215
  %67 = load i8, i8* %66, align 1, !dbg !215
  %68 = zext i8 %67 to i32, !dbg !215
  %69 = xor i32 %68, -1, !dbg !216
  %70 = trunc i32 %69 to i8, !dbg !216
  %71 = load i32, i32* %11, align 4, !dbg !217
  %72 = sext i32 %71 to i64, !dbg !218
  %73 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i64 0, i64 %72, !dbg !218
  store i8 %70, i8* %73, align 1, !dbg !219
  %74 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !220
  %75 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !221
  %76 = load i32, i32* %11, align 4, !dbg !222
  %77 = call i32 @BIO_read(%struct.bio_st* %74, i8* %75, i32 %76), !dbg !223
  store i32 %77, i32* %13, align 4, !dbg !224
  %78 = load i32, i32* %11, align 4, !dbg !225
  %79 = sext i32 %78 to i64, !dbg !225
  %80 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i64 0, i64 %79, !dbg !225
  %81 = load i8, i8* %80, align 1, !dbg !225
  %82 = load i32, i32* %11, align 4, !dbg !225
  %83 = sext i32 %82 to i64, !dbg !225
  %84 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i64 0, i64 %83, !dbg !225
  %85 = load i8, i8* %84, align 1, !dbg !225
  %86 = zext i8 %85 to i32, !dbg !225
  %87 = xor i32 %86, -1, !dbg !225
  %88 = trunc i32 %87 to i8, !dbg !225
  %89 = call i32 @test_uchar_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 72, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.11, i32 0, i32 0), i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.12, i32 0, i32 0), i8 zeroext %81, i8 zeroext %88), !dbg !225
  %90 = icmp ne i32 %89, 0, !dbg !225
  br i1 %90, label %93, label %91, !dbg !227

; <label>:91:                                     ; preds = %59
  %92 = load i32, i32* %11, align 4, !dbg !228
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 73, i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str.13, i32 0, i32 0), i32 %92), !dbg !228
  store i32 0, i32* %4, align 4, !dbg !230
  br label %342, !dbg !230

; <label>:93:                                     ; preds = %59
  %94 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !231
  %95 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !232
  %96 = load i32, i32* %13, align 4, !dbg !233
  %97 = sext i32 %96 to i64, !dbg !234
  %98 = getelementptr inbounds i8, i8* %95, i64 %97, !dbg !234
  %99 = load i32, i32* %13, align 4, !dbg !235
  %100 = sext i32 %99 to i64, !dbg !235
  %101 = sub i64 1056, %100, !dbg !236
  %102 = trunc i64 %101 to i32, !dbg !237
  %103 = call i32 @BIO_read(%struct.bio_st* %94, i8* %98, i32 %102), !dbg !238
  %104 = load i32, i32* %13, align 4, !dbg !239
  %105 = add nsw i32 %104, %103, !dbg !239
  store i32 %105, i32* %13, align 4, !dbg !239
  %106 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !240
  call void @BIO_free_all(%struct.bio_st* %106), !dbg !241
  %107 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !242
  %108 = load i32, i32* %13, align 4, !dbg !242
  %109 = sext i32 %108 to i64, !dbg !242
  %110 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i32 0, i32 0, !dbg !242
  %111 = load i32, i32* %12, align 4, !dbg !242
  %112 = sext i32 %111 to i64, !dbg !242
  %113 = call i32 @test_mem_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 79, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.15, i32 0, i32 0), i8* %107, i64 %109, i8* %110, i64 %112), !dbg !242
  %114 = icmp ne i32 %113, 0, !dbg !242
  br i1 %114, label %117, label %115, !dbg !244

; <label>:115:                                    ; preds = %93
  %116 = load i32, i32* %11, align 4, !dbg !245
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 80, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str.16, i32 0, i32 0), i32 %116), !dbg !245
  store i32 0, i32* %4, align 4, !dbg !247
  br label %342, !dbg !247

; <label>:117:                                    ; preds = %93
  br label %118, !dbg !248

; <label>:118:                                    ; preds = %117
  %119 = load i32, i32* %11, align 4, !dbg !249
  %120 = add nsw i32 %119, 1, !dbg !249
  store i32 %120, i32* %11, align 4, !dbg !249
  br label %41, !dbg !250, !llvm.loop !251

; <label>:121:                                    ; preds = %41
  store i32 1, i32* %11, align 4, !dbg !253
  br label %122, !dbg !255

; <label>:122:                                    ; preds = %172, %121
  %123 = load i32, i32* %11, align 4, !dbg !256
  %124 = load i32, i32* %12, align 4, !dbg !258
  %125 = sdiv i32 %124, 2, !dbg !259
  %126 = icmp slt i32 %123, %125, !dbg !260
  br i1 %126, label %127, label %175, !dbg !261

; <label>:127:                                    ; preds = %122
  call void @llvm.dbg.declare(metadata i32* %14, metadata !262, metadata !37), !dbg !264
  %128 = call %struct.bio_method_st* @BIO_f_cipher(), !dbg !265
  %129 = call %struct.bio_st* @BIO_new(%struct.bio_method_st* %128), !dbg !266
  store %struct.bio_st* %129, %struct.bio_st** %8, align 8, !dbg !267
  %130 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !268
  %131 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %5, align 8, !dbg !268
  %132 = load i8*, i8** %6, align 8, !dbg !268
  %133 = load i8*, i8** %7, align 8, !dbg !268
  %134 = call i32 @BIO_set_cipher(%struct.bio_st* %130, %struct.evp_cipher_st* %131, i8* %132, i8* %133, i32 1), !dbg !268
  %135 = icmp ne i32 %134, 0, !dbg !268
  %136 = zext i1 %135 to i32, !dbg !268
  %137 = call i32 @test_true(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 90, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.9, i32 0, i32 0), i32 %136), !dbg !268
  %138 = icmp ne i32 %137, 0, !dbg !268
  br i1 %138, label %141, label %139, !dbg !270

; <label>:139:                                    ; preds = %127
  %140 = load i32, i32* %11, align 4, !dbg !271
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 91, i8* getelementptr inbounds ([42 x i8], [42 x i8]* @.str.17, i32 0, i32 0), i32 %140), !dbg !271
  store i32 0, i32* %4, align 4, !dbg !273
  br label %342, !dbg !273

; <label>:141:                                    ; preds = %127
  %142 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !274
  %143 = call %struct.bio_st* @BIO_new_mem_buf(i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i32 1024), !dbg !275
  %144 = call %struct.bio_st* @BIO_push(%struct.bio_st* %142, %struct.bio_st* %143), !dbg !276
  %145 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !277
  call void @llvm.memset.p0i8.i64(i8* %145, i8 0, i64 1056, i32 16, i1 false), !dbg !277
  store i32 0, i32* %13, align 4, !dbg !278
  br label %146, !dbg !280

; <label>:146:                                    ; preds = %155, %141
  %147 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !281
  %148 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !283
  %149 = load i32, i32* %13, align 4, !dbg !284
  %150 = sext i32 %149 to i64, !dbg !285
  %151 = getelementptr inbounds i8, i8* %148, i64 %150, !dbg !285
  %152 = load i32, i32* %11, align 4, !dbg !286
  %153 = call i32 @BIO_read(%struct.bio_st* %147, i8* %151, i32 %152), !dbg !287
  store i32 %153, i32* %14, align 4, !dbg !288
  %154 = icmp ne i32 %153, 0, !dbg !289
  br i1 %154, label %155, label %159, !dbg !289

; <label>:155:                                    ; preds = %146
  %156 = load i32, i32* %14, align 4, !dbg !290
  %157 = load i32, i32* %13, align 4, !dbg !292
  %158 = add nsw i32 %157, %156, !dbg !292
  store i32 %158, i32* %13, align 4, !dbg !292
  br label %146, !dbg !293, !llvm.loop !294

; <label>:159:                                    ; preds = %146
  %160 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !296
  call void @BIO_free_all(%struct.bio_st* %160), !dbg !297
  %161 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !298
  %162 = load i32, i32* %13, align 4, !dbg !298
  %163 = sext i32 %162 to i64, !dbg !298
  %164 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i32 0, i32 0, !dbg !298
  %165 = load i32, i32* %12, align 4, !dbg !298
  %166 = sext i32 %165 to i64, !dbg !298
  %167 = call i32 @test_mem_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 101, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.15, i32 0, i32 0), i8* %161, i64 %163, i8* %164, i64 %166), !dbg !298
  %168 = icmp ne i32 %167, 0, !dbg !298
  br i1 %168, label %171, label %169, !dbg !300

; <label>:169:                                    ; preds = %159
  %170 = load i32, i32* %11, align 4, !dbg !301
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 102, i8* getelementptr inbounds ([50 x i8], [50 x i8]* @.str.18, i32 0, i32 0), i32 %170), !dbg !301
  store i32 0, i32* %4, align 4, !dbg !303
  br label %342, !dbg !303

; <label>:171:                                    ; preds = %159
  br label %172, !dbg !304

; <label>:172:                                    ; preds = %171
  %173 = load i32, i32* %11, align 4, !dbg !305
  %174 = add nsw i32 %173, 1, !dbg !305
  store i32 %174, i32* %11, align 4, !dbg !305
  br label %122, !dbg !306, !llvm.loop !307

; <label>:175:                                    ; preds = %122
  %176 = call %struct.bio_method_st* @BIO_f_cipher(), !dbg !309
  %177 = call %struct.bio_st* @BIO_new(%struct.bio_method_st* %176), !dbg !310
  store %struct.bio_st* %177, %struct.bio_st** %8, align 8, !dbg !311
  %178 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !312
  %179 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %5, align 8, !dbg !312
  %180 = load i8*, i8** %6, align 8, !dbg !312
  %181 = load i8*, i8** %7, align 8, !dbg !312
  %182 = call i32 @BIO_set_cipher(%struct.bio_st* %178, %struct.evp_cipher_st* %179, i8* %180, i8* %181, i32 0), !dbg !312
  %183 = icmp ne i32 %182, 0, !dbg !312
  %184 = zext i1 %183 to i32, !dbg !312
  %185 = call i32 @test_true(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 111, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.19, i32 0, i32 0), i32 %184), !dbg !312
  %186 = icmp ne i32 %185, 0, !dbg !312
  br i1 %186, label %188, label %187, !dbg !314

; <label>:187:                                    ; preds = %175
  store i32 0, i32* %4, align 4, !dbg !315
  br label %342, !dbg !315

; <label>:188:                                    ; preds = %175
  %189 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !316
  %190 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i32 0, i32 0, !dbg !317
  %191 = load i32, i32* %12, align 4, !dbg !318
  %192 = call %struct.bio_st* @BIO_new_mem_buf(i8* %190, i32 %191), !dbg !319
  %193 = call %struct.bio_st* @BIO_push(%struct.bio_st* %189, %struct.bio_st* %192), !dbg !320
  %194 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !321
  %195 = call i64 @BIO_ctrl(%struct.bio_st* %194, i32 11, i64 0, i8* null), !dbg !321
  %196 = trunc i64 %195 to i32, !dbg !321
  %197 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !322
  call void @llvm.memset.p0i8.i64(i8* %197, i8 0, i64 1056, i32 16, i1 false), !dbg !322
  %198 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !323
  %199 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !324
  %200 = call i32 @BIO_read(%struct.bio_st* %198, i8* %199, i32 1056), !dbg !325
  store i32 %200, i32* %13, align 4, !dbg !326
  %201 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !327
  call void @BIO_free_all(%struct.bio_st* %201), !dbg !328
  %202 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !329
  %203 = load i32, i32* %13, align 4, !dbg !329
  %204 = sext i32 %203 to i64, !dbg !329
  %205 = call i32 @test_mem_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 120, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.20, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i64 1024, i8* %202, i64 %204), !dbg !329
  %206 = icmp ne i32 %205, 0, !dbg !329
  br i1 %206, label %208, label %207, !dbg !331

; <label>:207:                                    ; preds = %188
  store i32 0, i32* %4, align 4, !dbg !332
  br label %342, !dbg !332

; <label>:208:                                    ; preds = %188
  store i32 1, i32* %11, align 4, !dbg !333
  br label %209, !dbg !335

; <label>:209:                                    ; preds = %285, %208
  %210 = load i32, i32* %11, align 4, !dbg !336
  %211 = load i32, i32* %12, align 4, !dbg !338
  %212 = icmp slt i32 %210, %211, !dbg !339
  br i1 %212, label %213, label %288, !dbg !340

; <label>:213:                                    ; preds = %209
  %214 = call %struct.bio_method_st* @BIO_f_cipher(), !dbg !341
  %215 = call %struct.bio_st* @BIO_new(%struct.bio_method_st* %214), !dbg !343
  store %struct.bio_st* %215, %struct.bio_st** %8, align 8, !dbg !344
  %216 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !345
  %217 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %5, align 8, !dbg !345
  %218 = load i8*, i8** %6, align 8, !dbg !345
  %219 = load i8*, i8** %7, align 8, !dbg !345
  %220 = call i32 @BIO_set_cipher(%struct.bio_st* %216, %struct.evp_cipher_st* %217, i8* %218, i8* %219, i32 0), !dbg !345
  %221 = icmp ne i32 %220, 0, !dbg !345
  %222 = zext i1 %221 to i32, !dbg !345
  %223 = call i32 @test_true(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 126, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.19, i32 0, i32 0), i32 %222), !dbg !345
  %224 = icmp ne i32 %223, 0, !dbg !345
  br i1 %224, label %227, label %225, !dbg !347

; <label>:225:                                    ; preds = %213
  %226 = load i32, i32* %11, align 4, !dbg !348
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 127, i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.21, i32 0, i32 0), i32 %226), !dbg !348
  store i32 0, i32* %4, align 4, !dbg !350
  br label %342, !dbg !350

; <label>:227:                                    ; preds = %213
  %228 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !351
  %229 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i32 0, i32 0, !dbg !352
  %230 = load i32, i32* %12, align 4, !dbg !353
  %231 = call %struct.bio_st* @BIO_new_mem_buf(i8* %229, i32 %230), !dbg !354
  %232 = call %struct.bio_st* @BIO_push(%struct.bio_st* %228, %struct.bio_st* %231), !dbg !355
  %233 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !356
  call void @llvm.memset.p0i8.i64(i8* %233, i8 0, i64 1056, i32 16, i1 false), !dbg !356
  %234 = load i32, i32* %11, align 4, !dbg !357
  %235 = sext i32 %234 to i64, !dbg !358
  %236 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i64 0, i64 %235, !dbg !358
  %237 = load i8, i8* %236, align 1, !dbg !358
  %238 = zext i8 %237 to i32, !dbg !358
  %239 = xor i32 %238, -1, !dbg !359
  %240 = trunc i32 %239 to i8, !dbg !359
  %241 = load i32, i32* %11, align 4, !dbg !360
  %242 = sext i32 %241 to i64, !dbg !361
  %243 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i64 0, i64 %242, !dbg !361
  store i8 %240, i8* %243, align 1, !dbg !362
  %244 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !363
  %245 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !364
  %246 = load i32, i32* %11, align 4, !dbg !365
  %247 = call i32 @BIO_read(%struct.bio_st* %244, i8* %245, i32 %246), !dbg !366
  store i32 %247, i32* %13, align 4, !dbg !367
  %248 = load i32, i32* %11, align 4, !dbg !368
  %249 = sext i32 %248 to i64, !dbg !368
  %250 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i64 0, i64 %249, !dbg !368
  %251 = load i8, i8* %250, align 1, !dbg !368
  %252 = load i32, i32* %11, align 4, !dbg !368
  %253 = sext i32 %252 to i64, !dbg !368
  %254 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i64 0, i64 %253, !dbg !368
  %255 = load i8, i8* %254, align 1, !dbg !368
  %256 = zext i8 %255 to i32, !dbg !368
  %257 = xor i32 %256, -1, !dbg !368
  %258 = trunc i32 %257 to i8, !dbg !368
  %259 = call i32 @test_uchar_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 135, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.11, i32 0, i32 0), i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.12, i32 0, i32 0), i8 zeroext %251, i8 zeroext %258), !dbg !368
  %260 = icmp ne i32 %259, 0, !dbg !368
  br i1 %260, label %263, label %261, !dbg !370

; <label>:261:                                    ; preds = %227
  %262 = load i32, i32* %11, align 4, !dbg !371
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 136, i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str.22, i32 0, i32 0), i32 %262), !dbg !371
  store i32 0, i32* %4, align 4, !dbg !373
  br label %342, !dbg !373

; <label>:263:                                    ; preds = %227
  %264 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !374
  %265 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !375
  %266 = load i32, i32* %13, align 4, !dbg !376
  %267 = sext i32 %266 to i64, !dbg !377
  %268 = getelementptr inbounds i8, i8* %265, i64 %267, !dbg !377
  %269 = load i32, i32* %13, align 4, !dbg !378
  %270 = sext i32 %269 to i64, !dbg !378
  %271 = sub i64 1056, %270, !dbg !379
  %272 = trunc i64 %271 to i32, !dbg !380
  %273 = call i32 @BIO_read(%struct.bio_st* %264, i8* %268, i32 %272), !dbg !381
  %274 = load i32, i32* %13, align 4, !dbg !382
  %275 = add nsw i32 %274, %273, !dbg !382
  store i32 %275, i32* %13, align 4, !dbg !382
  %276 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !383
  call void @BIO_free_all(%struct.bio_st* %276), !dbg !384
  %277 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !385
  %278 = load i32, i32* %13, align 4, !dbg !385
  %279 = sext i32 %278 to i64, !dbg !385
  %280 = call i32 @test_mem_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 142, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.20, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i64 1024, i8* %277, i64 %279), !dbg !385
  %281 = icmp ne i32 %280, 0, !dbg !385
  br i1 %281, label %284, label %282, !dbg !387

; <label>:282:                                    ; preds = %263
  %283 = load i32, i32* %11, align 4, !dbg !388
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 143, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str.23, i32 0, i32 0), i32 %283), !dbg !388
  store i32 0, i32* %4, align 4, !dbg !390
  br label %342, !dbg !390

; <label>:284:                                    ; preds = %263
  br label %285, !dbg !391

; <label>:285:                                    ; preds = %284
  %286 = load i32, i32* %11, align 4, !dbg !392
  %287 = add nsw i32 %286, 1, !dbg !392
  store i32 %287, i32* %11, align 4, !dbg !392
  br label %209, !dbg !393, !llvm.loop !394

; <label>:288:                                    ; preds = %209
  store i32 1, i32* %11, align 4, !dbg !396
  br label %289, !dbg !398

; <label>:289:                                    ; preds = %338, %288
  %290 = load i32, i32* %11, align 4, !dbg !399
  %291 = load i32, i32* %12, align 4, !dbg !401
  %292 = sdiv i32 %291, 2, !dbg !402
  %293 = icmp slt i32 %290, %292, !dbg !403
  br i1 %293, label %294, label %341, !dbg !404

; <label>:294:                                    ; preds = %289
  call void @llvm.dbg.declare(metadata i32* %15, metadata !405, metadata !37), !dbg !407
  %295 = call %struct.bio_method_st* @BIO_f_cipher(), !dbg !408
  %296 = call %struct.bio_st* @BIO_new(%struct.bio_method_st* %295), !dbg !409
  store %struct.bio_st* %296, %struct.bio_st** %8, align 8, !dbg !410
  %297 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !411
  %298 = load %struct.evp_cipher_st*, %struct.evp_cipher_st** %5, align 8, !dbg !411
  %299 = load i8*, i8** %6, align 8, !dbg !411
  %300 = load i8*, i8** %7, align 8, !dbg !411
  %301 = call i32 @BIO_set_cipher(%struct.bio_st* %297, %struct.evp_cipher_st* %298, i8* %299, i8* %300, i32 0), !dbg !411
  %302 = icmp ne i32 %301, 0, !dbg !411
  %303 = zext i1 %302 to i32, !dbg !411
  %304 = call i32 @test_true(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 153, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.19, i32 0, i32 0), i32 %303), !dbg !411
  %305 = icmp ne i32 %304, 0, !dbg !411
  br i1 %305, label %308, label %306, !dbg !413

; <label>:306:                                    ; preds = %294
  %307 = load i32, i32* %11, align 4, !dbg !414
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 154, i8* getelementptr inbounds ([42 x i8], [42 x i8]* @.str.24, i32 0, i32 0), i32 %307), !dbg !414
  store i32 0, i32* %4, align 4, !dbg !416
  br label %342, !dbg !416

; <label>:308:                                    ; preds = %294
  %309 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !417
  %310 = getelementptr inbounds [1056 x i8], [1056 x i8]* %10, i32 0, i32 0, !dbg !418
  %311 = load i32, i32* %12, align 4, !dbg !419
  %312 = call %struct.bio_st* @BIO_new_mem_buf(i8* %310, i32 %311), !dbg !420
  %313 = call %struct.bio_st* @BIO_push(%struct.bio_st* %309, %struct.bio_st* %312), !dbg !421
  %314 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !422
  call void @llvm.memset.p0i8.i64(i8* %314, i8 0, i64 1056, i32 16, i1 false), !dbg !422
  store i32 0, i32* %13, align 4, !dbg !423
  br label %315, !dbg !425

; <label>:315:                                    ; preds = %324, %308
  %316 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !426
  %317 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !428
  %318 = load i32, i32* %13, align 4, !dbg !429
  %319 = sext i32 %318 to i64, !dbg !430
  %320 = getelementptr inbounds i8, i8* %317, i64 %319, !dbg !430
  %321 = load i32, i32* %11, align 4, !dbg !431
  %322 = call i32 @BIO_read(%struct.bio_st* %316, i8* %320, i32 %321), !dbg !432
  store i32 %322, i32* %15, align 4, !dbg !433
  %323 = icmp ne i32 %322, 0, !dbg !434
  br i1 %323, label %324, label %328, !dbg !434

; <label>:324:                                    ; preds = %315
  %325 = load i32, i32* %15, align 4, !dbg !435
  %326 = load i32, i32* %13, align 4, !dbg !437
  %327 = add nsw i32 %326, %325, !dbg !437
  store i32 %327, i32* %13, align 4, !dbg !437
  br label %315, !dbg !438, !llvm.loop !439

; <label>:328:                                    ; preds = %315
  %329 = load %struct.bio_st*, %struct.bio_st** %8, align 8, !dbg !441
  call void @BIO_free_all(%struct.bio_st* %329), !dbg !442
  %330 = getelementptr inbounds [1056 x i8], [1056 x i8]* %9, i32 0, i32 0, !dbg !443
  %331 = load i32, i32* %13, align 4, !dbg !443
  %332 = sext i32 %331 to i64, !dbg !443
  %333 = call i32 @test_mem_eq(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 164, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.20, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds ([1056 x i8], [1056 x i8]* @do_bio_cipher.inp, i32 0, i32 0), i64 1024, i8* %330, i64 %332), !dbg !443
  %334 = icmp ne i32 %333, 0, !dbg !443
  br i1 %334, label %337, label %335, !dbg !445

; <label>:335:                                    ; preds = %328
  %336 = load i32, i32* %11, align 4, !dbg !446
  call void (i8*, i32, i8*, ...) @test_info(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i32 0, i32 0), i32 165, i8* getelementptr inbounds ([50 x i8], [50 x i8]* @.str.25, i32 0, i32 0), i32 %336), !dbg !446
  store i32 0, i32* %4, align 4, !dbg !448
  br label %342, !dbg !448

; <label>:337:                                    ; preds = %328
  br label %338, !dbg !449

; <label>:338:                                    ; preds = %337
  %339 = load i32, i32* %11, align 4, !dbg !450
  %340 = add nsw i32 %339, 1, !dbg !450
  store i32 %340, i32* %11, align 4, !dbg !450
  br label %289, !dbg !451, !llvm.loop !452

; <label>:341:                                    ; preds = %289
  store i32 1, i32* %4, align 4, !dbg !454
  br label %342, !dbg !454

; <label>:342:                                    ; preds = %341, %335, %306, %282, %261, %225, %207, %187, %169, %139, %115, %91, %57, %32, %19
  %343 = load i32, i32* %4, align 4, !dbg !455
  ret i32 %343, !dbg !455
}

declare i32 @test_int_gt(i8*, i32, i8*, i8*, i32, i32) #2

declare i32 @RAND_bytes(i8*, i32) #2

declare %struct.bio_st* @BIO_new(%struct.bio_method_st*) #2

declare %struct.bio_method_st* @BIO_f_cipher() #2

declare i32 @test_true(i8*, i32, i8*, i32) #2

declare i32 @BIO_set_cipher(%struct.bio_st*, %struct.evp_cipher_st*, i8*, i8*, i32) #2

declare %struct.bio_st* @BIO_push(%struct.bio_st*, %struct.bio_st*) #2

declare %struct.bio_st* @BIO_new_mem_buf(i8*, i32) #2

declare i32 @BIO_read(%struct.bio_st*, i8*, i32) #2

declare void @BIO_free_all(%struct.bio_st*) #2

declare void @test_info(i8*, i32, i8*, ...) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #3

declare i32 @test_uchar_eq(i8*, i32, i8*, i8*, i8 zeroext, i8 zeroext) #2

declare i32 @test_mem_eq(i8*, i32, i8*, i8*, i8*, i64, i8*, i64) #2

declare i64 @BIO_ctrl(%struct.bio_st*, i32, i64, i8*) #2

declare %struct.evp_cipher_st* @EVP_aes_128_ctr() #2

declare %struct.evp_cipher_st* @EVP_aes_256_cfb128() #2

declare %struct.evp_cipher_st* @EVP_aes_256_ofb() #2

declare %struct.evp_cipher_st* @EVP_chacha20() #2

declare %struct.evp_cipher_st* @EVP_chacha20_poly1305() #2

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!27, !28, !29}
!llvm.ident = !{!30}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.9.1 (tags/RELEASE_391/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, globals: !7)
!1 = !DIFile(filename: "test/bio_enc_test.c", directory: "/Users/alexdenisov/Projects/OpenSSL/openssl")
!2 = !{}
!3 = !{!4, !5, !6}
!4 = !DIBasicType(name: "unsigned char", size: 8, align: 8, encoding: DW_ATE_unsigned_char)
!5 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64, align: 64)
!7 = !{!8, !22, !26}
!8 = distinct !DIGlobalVariable(name: "inp", scope: !9, file: !1, line: 42, type: !19, isLocal: true, isDefinition: true, variable: [1056 x i8]* @do_bio_cipher.inp)
!9 = distinct !DISubprogram(name: "do_bio_cipher", scope: !1, file: !1, line: 38, type: !10, isLocal: true, isDefinition: true, scopeLine: 40, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!10 = !DISubroutineType(types: !11)
!11 = !{!5, !12, !17, !17}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64, align: 64)
!13 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !14)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "EVP_CIPHER", file: !15, line: 89, baseType: !16)
!15 = !DIFile(filename: "include/openssl/ossl_typ.h", directory: "/Users/alexdenisov/Projects/OpenSSL/openssl")
!16 = !DICompositeType(tag: DW_TAG_structure_type, name: "evp_cipher_st", file: !15, line: 89, flags: DIFlagFwdDecl)
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64, align: 64)
!18 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !4)
!19 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 8448, align: 8, elements: !20)
!20 = !{!21}
!21 = !DISubrange(count: 1056)
!22 = distinct !DIGlobalVariable(name: "KEY", scope: !0, file: !1, line: 24, type: !23, isLocal: true, isDefinition: true, variable: [32 x i8]* @KEY)
!23 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, size: 256, align: 8, elements: !24)
!24 = !{!25}
!25 = !DISubrange(count: 32)
!26 = distinct !DIGlobalVariable(name: "IV", scope: !0, file: !1, line: 31, type: !23, isLocal: true, isDefinition: true, variable: [32 x i8]* @IV)
!27 = !{i32 2, !"Dwarf Version", i32 2}
!28 = !{i32 2, !"Debug Info Version", i32 3}
!29 = !{i32 1, !"PIC Level", i32 2}
!30 = !{!"clang version 3.9.1 (tags/RELEASE_391/final)"}
!31 = distinct !DISubprogram(name: "setup_single_tests", scope: !1, file: !1, line: 229, type: !32, isLocal: false, isDefinition: true, scopeLine: 230, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!32 = !DISubroutineType(types: !33)
!33 = !{null}
!34 = !DILocalVariable(name: "test_name", scope: !31, file: !1, line: 231, type: !35)
!35 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !36, size: 64, align: 64)
!36 = !DIBasicType(name: "char", size: 8, align: 8, encoding: DW_ATE_signed_char)
!37 = !DIExpression()
!38 = !DILocation(line: 231, column: 9, scope: !31)
!39 = !DILocation(line: 231, column: 21, scope: !31)
!40 = !DILocation(line: 233, column: 3, scope: !41)
!41 = distinct !DILexicalBlock(scope: !31, file: !1, line: 233, column: 3)
!42 = !DILocation(line: 233, column: 3, scope: !31)
!43 = !DILocation(line: 233, column: 3, scope: !44)
!44 = distinct !DILexicalBlock(scope: !41, file: !1, line: 233, column: 3)
!45 = !DILocation(line: 234, column: 3, scope: !46)
!46 = distinct !DILexicalBlock(scope: !31, file: !1, line: 234, column: 3)
!47 = !DILocation(line: 234, column: 3, scope: !31)
!48 = !DILocation(line: 234, column: 3, scope: !49)
!49 = distinct !DILexicalBlock(scope: !46, file: !1, line: 234, column: 3)
!50 = !DILocation(line: 235, column: 3, scope: !51)
!51 = distinct !DILexicalBlock(scope: !31, file: !1, line: 235, column: 3)
!52 = !DILocation(line: 235, column: 3, scope: !31)
!53 = !DILocation(line: 235, column: 3, scope: !54)
!54 = distinct !DILexicalBlock(scope: !51, file: !1, line: 235, column: 3)
!55 = !DILocation(line: 236, column: 3, scope: !56)
!56 = distinct !DILexicalBlock(scope: !31, file: !1, line: 236, column: 3)
!57 = !DILocation(line: 236, column: 3, scope: !31)
!58 = !DILocation(line: 236, column: 3, scope: !59)
!59 = distinct !DILexicalBlock(scope: !56, file: !1, line: 236, column: 3)
!60 = !DILocation(line: 237, column: 3, scope: !61)
!61 = distinct !DILexicalBlock(scope: !31, file: !1, line: 237, column: 3)
!62 = !DILocation(line: 237, column: 3, scope: !31)
!63 = !DILocation(line: 237, column: 3, scope: !64)
!64 = distinct !DILexicalBlock(scope: !61, file: !1, line: 237, column: 3)
!65 = !DILocation(line: 238, column: 3, scope: !66)
!66 = distinct !DILexicalBlock(scope: !31, file: !1, line: 238, column: 3)
!67 = !DILocation(line: 238, column: 3, scope: !31)
!68 = !DILocation(line: 238, column: 3, scope: !69)
!69 = distinct !DILexicalBlock(scope: !66, file: !1, line: 238, column: 3)
!70 = !DILocation(line: 239, column: 1, scope: !31)
!71 = distinct !DISubprogram(name: "test_bio_enc_aes_128_cbc", scope: !1, file: !1, line: 185, type: !72, isLocal: true, isDefinition: true, scopeLine: 186, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!72 = !DISubroutineType(types: !73)
!73 = !{!5, !5}
!74 = !DILocalVariable(name: "idx", arg: 1, scope: !71, file: !1, line: 185, type: !5)
!75 = !DILocation(line: 185, column: 41, scope: !71)
!76 = !DILocation(line: 187, column: 31, scope: !71)
!77 = !DILocation(line: 187, column: 50, scope: !71)
!78 = !DILocation(line: 187, column: 12, scope: !71)
!79 = !DILocation(line: 187, column: 5, scope: !71)
!80 = distinct !DISubprogram(name: "test_bio_enc_aes_128_ctr", scope: !1, file: !1, line: 190, type: !72, isLocal: true, isDefinition: true, scopeLine: 191, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!81 = !DILocalVariable(name: "idx", arg: 1, scope: !80, file: !1, line: 190, type: !5)
!82 = !DILocation(line: 190, column: 41, scope: !80)
!83 = !DILocation(line: 192, column: 31, scope: !80)
!84 = !DILocation(line: 192, column: 50, scope: !80)
!85 = !DILocation(line: 192, column: 12, scope: !80)
!86 = !DILocation(line: 192, column: 5, scope: !80)
!87 = distinct !DISubprogram(name: "test_bio_enc_aes_256_cfb", scope: !1, file: !1, line: 195, type: !72, isLocal: true, isDefinition: true, scopeLine: 196, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!88 = !DILocalVariable(name: "idx", arg: 1, scope: !87, file: !1, line: 195, type: !5)
!89 = !DILocation(line: 195, column: 41, scope: !87)
!90 = !DILocation(line: 197, column: 31, scope: !87)
!91 = !DILocation(line: 197, column: 50, scope: !87)
!92 = !DILocation(line: 197, column: 12, scope: !87)
!93 = !DILocation(line: 197, column: 5, scope: !87)
!94 = distinct !DISubprogram(name: "test_bio_enc_aes_256_ofb", scope: !1, file: !1, line: 200, type: !72, isLocal: true, isDefinition: true, scopeLine: 201, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!95 = !DILocalVariable(name: "idx", arg: 1, scope: !94, file: !1, line: 200, type: !5)
!96 = !DILocation(line: 200, column: 41, scope: !94)
!97 = !DILocation(line: 202, column: 31, scope: !94)
!98 = !DILocation(line: 202, column: 50, scope: !94)
!99 = !DILocation(line: 202, column: 12, scope: !94)
!100 = !DILocation(line: 202, column: 5, scope: !94)
!101 = distinct !DISubprogram(name: "test_bio_enc_chacha20", scope: !1, file: !1, line: 206, type: !72, isLocal: true, isDefinition: true, scopeLine: 207, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!102 = !DILocalVariable(name: "idx", arg: 1, scope: !101, file: !1, line: 206, type: !5)
!103 = !DILocation(line: 206, column: 38, scope: !101)
!104 = !DILocation(line: 208, column: 31, scope: !101)
!105 = !DILocation(line: 208, column: 47, scope: !101)
!106 = !DILocation(line: 208, column: 12, scope: !101)
!107 = !DILocation(line: 208, column: 5, scope: !101)
!108 = distinct !DISubprogram(name: "test_bio_enc_chacha20_poly1305", scope: !1, file: !1, line: 212, type: !72, isLocal: true, isDefinition: true, scopeLine: 213, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!109 = !DILocalVariable(name: "idx", arg: 1, scope: !108, file: !1, line: 212, type: !5)
!110 = !DILocation(line: 212, column: 47, scope: !108)
!111 = !DILocation(line: 214, column: 31, scope: !108)
!112 = !DILocation(line: 214, column: 56, scope: !108)
!113 = !DILocation(line: 214, column: 12, scope: !108)
!114 = !DILocation(line: 214, column: 5, scope: !108)
!115 = distinct !DISubprogram(name: "setup_tests", scope: !1, file: !1, line: 241, type: !116, isLocal: false, isDefinition: true, scopeLine: 242, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!116 = !DISubroutineType(types: !117)
!117 = !{!5}
!118 = !DILocation(line: 243, column: 9, scope: !119)
!119 = distinct !DILexicalBlock(scope: !115, file: !1, line: 243, column: 9)
!120 = !DILocation(line: 243, column: 35, scope: !119)
!121 = !DILocation(line: 243, column: 9, scope: !115)
!122 = !DILocation(line: 244, column: 7, scope: !123)
!123 = distinct !DILexicalBlock(scope: !119, file: !1, line: 243, column: 40)
!124 = !DILocation(line: 245, column: 7, scope: !123)
!125 = !DILocation(line: 247, column: 5, scope: !115)
!126 = !DILocation(line: 248, column: 5, scope: !115)
!127 = !DILocation(line: 249, column: 5, scope: !115)
!128 = !DILocation(line: 250, column: 5, scope: !115)
!129 = !DILocation(line: 252, column: 5, scope: !115)
!130 = !DILocation(line: 254, column: 5, scope: !115)
!131 = !DILocation(line: 257, column: 5, scope: !115)
!132 = !DILocation(line: 258, column: 1, scope: !115)
!133 = distinct !DISubprogram(name: "do_test_bio_cipher", scope: !1, file: !1, line: 173, type: !134, isLocal: true, isDefinition: true, scopeLine: 174, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!134 = !DISubroutineType(types: !135)
!135 = !{!5, !12, !5}
!136 = !DILocalVariable(name: "cipher", arg: 1, scope: !133, file: !1, line: 173, type: !12)
!137 = !DILocation(line: 173, column: 49, scope: !133)
!138 = !DILocalVariable(name: "idx", arg: 2, scope: !133, file: !1, line: 173, type: !5)
!139 = !DILocation(line: 173, column: 61, scope: !133)
!140 = !DILocation(line: 175, column: 12, scope: !133)
!141 = !DILocation(line: 175, column: 5, scope: !133)
!142 = !DILocation(line: 178, column: 34, scope: !143)
!143 = distinct !DILexicalBlock(scope: !133, file: !1, line: 176, column: 5)
!144 = !DILocation(line: 178, column: 20, scope: !143)
!145 = !DILocation(line: 178, column: 13, scope: !143)
!146 = !DILocation(line: 180, column: 34, scope: !143)
!147 = !DILocation(line: 180, column: 20, scope: !143)
!148 = !DILocation(line: 180, column: 13, scope: !143)
!149 = !DILocation(line: 182, column: 5, scope: !133)
!150 = !DILocation(line: 183, column: 1, scope: !133)
!151 = !DILocalVariable(name: "cipher", arg: 1, scope: !9, file: !1, line: 38, type: !12)
!152 = !DILocation(line: 38, column: 44, scope: !9)
!153 = !DILocalVariable(name: "key", arg: 2, scope: !9, file: !1, line: 38, type: !17)
!154 = !DILocation(line: 38, column: 73, scope: !9)
!155 = !DILocalVariable(name: "iv", arg: 3, scope: !9, file: !1, line: 39, type: !17)
!156 = !DILocation(line: 39, column: 26, scope: !9)
!157 = !DILocalVariable(name: "b", scope: !9, file: !1, line: 41, type: !158)
!158 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !159, size: 64, align: 64)
!159 = !DIDerivedType(tag: DW_TAG_typedef, name: "BIO", file: !15, line: 79, baseType: !160)
!160 = !DICompositeType(tag: DW_TAG_structure_type, name: "bio_st", file: !15, line: 79, flags: DIFlagFwdDecl)
!161 = !DILocation(line: 41, column: 10, scope: !9)
!162 = !DILocalVariable(name: "out", scope: !9, file: !1, line: 43, type: !19)
!163 = !DILocation(line: 43, column: 19, scope: !9)
!164 = !DILocalVariable(name: "ref", scope: !9, file: !1, line: 43, type: !19)
!165 = !DILocation(line: 43, column: 34, scope: !9)
!166 = !DILocalVariable(name: "i", scope: !9, file: !1, line: 44, type: !5)
!167 = !DILocation(line: 44, column: 9, scope: !9)
!168 = !DILocalVariable(name: "lref", scope: !9, file: !1, line: 44, type: !5)
!169 = !DILocation(line: 44, column: 12, scope: !9)
!170 = !DILocalVariable(name: "len", scope: !9, file: !1, line: 44, type: !5)
!171 = !DILocation(line: 44, column: 18, scope: !9)
!172 = !DILocation(line: 47, column: 10, scope: !173)
!173 = distinct !DILexicalBlock(scope: !9, file: !1, line: 47, column: 9)
!174 = !DILocation(line: 47, column: 9, scope: !9)
!175 = !DILocation(line: 48, column: 9, scope: !173)
!176 = !DILocation(line: 53, column: 17, scope: !9)
!177 = !DILocation(line: 53, column: 9, scope: !9)
!178 = !DILocation(line: 53, column: 7, scope: !9)
!179 = !DILocation(line: 54, column: 10, scope: !180)
!180 = distinct !DILexicalBlock(scope: !9, file: !1, line: 54, column: 9)
!181 = !DILocation(line: 54, column: 9, scope: !9)
!182 = !DILocation(line: 55, column: 9, scope: !180)
!183 = !DILocation(line: 56, column: 14, scope: !9)
!184 = !DILocation(line: 56, column: 17, scope: !9)
!185 = !DILocation(line: 56, column: 5, scope: !9)
!186 = !DILocation(line: 57, column: 21, scope: !9)
!187 = !DILocation(line: 57, column: 24, scope: !9)
!188 = !DILocation(line: 57, column: 12, scope: !9)
!189 = !DILocation(line: 57, column: 10, scope: !9)
!190 = !DILocation(line: 58, column: 18, scope: !9)
!191 = !DILocation(line: 58, column: 5, scope: !9)
!192 = !DILocation(line: 61, column: 12, scope: !193)
!193 = distinct !DILexicalBlock(scope: !9, file: !1, line: 61, column: 5)
!194 = !DILocation(line: 61, column: 10, scope: !193)
!195 = !DILocation(line: 61, column: 17, scope: !196)
!196 = distinct !DILexicalBlock(scope: !193, file: !1, line: 61, column: 5)
!197 = !DILocation(line: 61, column: 21, scope: !196)
!198 = !DILocation(line: 61, column: 19, scope: !196)
!199 = !DILocation(line: 61, column: 5, scope: !193)
!200 = !DILocation(line: 62, column: 21, scope: !201)
!201 = distinct !DILexicalBlock(scope: !196, file: !1, line: 61, column: 32)
!202 = !DILocation(line: 62, column: 13, scope: !201)
!203 = !DILocation(line: 62, column: 11, scope: !201)
!204 = !DILocation(line: 63, column: 14, scope: !205)
!205 = distinct !DILexicalBlock(scope: !201, file: !1, line: 63, column: 13)
!206 = !DILocation(line: 63, column: 13, scope: !201)
!207 = !DILocation(line: 64, column: 13, scope: !208)
!208 = distinct !DILexicalBlock(scope: !205, file: !1, line: 63, column: 70)
!209 = !DILocation(line: 65, column: 13, scope: !208)
!210 = !DILocation(line: 67, column: 18, scope: !201)
!211 = !DILocation(line: 67, column: 21, scope: !201)
!212 = !DILocation(line: 67, column: 9, scope: !201)
!213 = !DILocation(line: 68, column: 9, scope: !201)
!214 = !DILocation(line: 69, column: 23, scope: !201)
!215 = !DILocation(line: 69, column: 19, scope: !201)
!216 = !DILocation(line: 69, column: 18, scope: !201)
!217 = !DILocation(line: 69, column: 13, scope: !201)
!218 = !DILocation(line: 69, column: 9, scope: !201)
!219 = !DILocation(line: 69, column: 16, scope: !201)
!220 = !DILocation(line: 70, column: 24, scope: !201)
!221 = !DILocation(line: 70, column: 27, scope: !201)
!222 = !DILocation(line: 70, column: 32, scope: !201)
!223 = !DILocation(line: 70, column: 15, scope: !201)
!224 = !DILocation(line: 70, column: 13, scope: !201)
!225 = !DILocation(line: 72, column: 14, scope: !226)
!226 = distinct !DILexicalBlock(scope: !201, file: !1, line: 72, column: 13)
!227 = !DILocation(line: 72, column: 13, scope: !201)
!228 = !DILocation(line: 73, column: 13, scope: !229)
!229 = distinct !DILexicalBlock(scope: !226, file: !1, line: 72, column: 61)
!230 = !DILocation(line: 74, column: 13, scope: !229)
!231 = !DILocation(line: 76, column: 25, scope: !201)
!232 = !DILocation(line: 76, column: 28, scope: !201)
!233 = !DILocation(line: 76, column: 34, scope: !201)
!234 = !DILocation(line: 76, column: 32, scope: !201)
!235 = !DILocation(line: 76, column: 53, scope: !201)
!236 = !DILocation(line: 76, column: 51, scope: !201)
!237 = !DILocation(line: 76, column: 39, scope: !201)
!238 = !DILocation(line: 76, column: 16, scope: !201)
!239 = !DILocation(line: 76, column: 13, scope: !201)
!240 = !DILocation(line: 77, column: 22, scope: !201)
!241 = !DILocation(line: 77, column: 9, scope: !201)
!242 = !DILocation(line: 79, column: 14, scope: !243)
!243 = distinct !DILexicalBlock(scope: !201, file: !1, line: 79, column: 13)
!244 = !DILocation(line: 79, column: 13, scope: !201)
!245 = !DILocation(line: 80, column: 13, scope: !246)
!246 = distinct !DILexicalBlock(scope: !243, file: !1, line: 79, column: 48)
!247 = !DILocation(line: 81, column: 13, scope: !246)
!248 = !DILocation(line: 83, column: 5, scope: !201)
!249 = !DILocation(line: 61, column: 28, scope: !196)
!250 = !DILocation(line: 61, column: 5, scope: !196)
!251 = distinct !{!251, !252}
!252 = !DILocation(line: 61, column: 5, scope: !9)
!253 = !DILocation(line: 86, column: 12, scope: !254)
!254 = distinct !DILexicalBlock(scope: !9, file: !1, line: 86, column: 5)
!255 = !DILocation(line: 86, column: 10, scope: !254)
!256 = !DILocation(line: 86, column: 17, scope: !257)
!257 = distinct !DILexicalBlock(scope: !254, file: !1, line: 86, column: 5)
!258 = !DILocation(line: 86, column: 21, scope: !257)
!259 = !DILocation(line: 86, column: 26, scope: !257)
!260 = !DILocation(line: 86, column: 19, scope: !257)
!261 = !DILocation(line: 86, column: 5, scope: !254)
!262 = !DILocalVariable(name: "delta", scope: !263, file: !1, line: 87, type: !5)
!263 = distinct !DILexicalBlock(scope: !257, file: !1, line: 86, column: 36)
!264 = !DILocation(line: 87, column: 13, scope: !263)
!265 = !DILocation(line: 89, column: 21, scope: !263)
!266 = !DILocation(line: 89, column: 13, scope: !263)
!267 = !DILocation(line: 89, column: 11, scope: !263)
!268 = !DILocation(line: 90, column: 14, scope: !269)
!269 = distinct !DILexicalBlock(scope: !263, file: !1, line: 90, column: 13)
!270 = !DILocation(line: 90, column: 13, scope: !263)
!271 = !DILocation(line: 91, column: 13, scope: !272)
!272 = distinct !DILexicalBlock(scope: !269, file: !1, line: 90, column: 70)
!273 = !DILocation(line: 92, column: 13, scope: !272)
!274 = !DILocation(line: 94, column: 18, scope: !263)
!275 = !DILocation(line: 94, column: 21, scope: !263)
!276 = !DILocation(line: 94, column: 9, scope: !263)
!277 = !DILocation(line: 95, column: 9, scope: !263)
!278 = !DILocation(line: 96, column: 18, scope: !279)
!279 = distinct !DILexicalBlock(scope: !263, file: !1, line: 96, column: 9)
!280 = !DILocation(line: 96, column: 14, scope: !279)
!281 = !DILocation(line: 96, column: 41, scope: !282)
!282 = distinct !DILexicalBlock(scope: !279, file: !1, line: 96, column: 9)
!283 = !DILocation(line: 96, column: 44, scope: !282)
!284 = !DILocation(line: 96, column: 50, scope: !282)
!285 = !DILocation(line: 96, column: 48, scope: !282)
!286 = !DILocation(line: 96, column: 55, scope: !282)
!287 = !DILocation(line: 96, column: 32, scope: !282)
!288 = !DILocation(line: 96, column: 30, scope: !282)
!289 = !DILocation(line: 96, column: 9, scope: !279)
!290 = !DILocation(line: 97, column: 20, scope: !291)
!291 = distinct !DILexicalBlock(scope: !282, file: !1, line: 96, column: 62)
!292 = !DILocation(line: 97, column: 17, scope: !291)
!293 = !DILocation(line: 96, column: 9, scope: !282)
!294 = distinct !{!294, !295}
!295 = !DILocation(line: 96, column: 9, scope: !263)
!296 = !DILocation(line: 99, column: 22, scope: !263)
!297 = !DILocation(line: 99, column: 9, scope: !263)
!298 = !DILocation(line: 101, column: 14, scope: !299)
!299 = distinct !DILexicalBlock(scope: !263, file: !1, line: 101, column: 13)
!300 = !DILocation(line: 101, column: 13, scope: !263)
!301 = !DILocation(line: 102, column: 13, scope: !302)
!302 = distinct !DILexicalBlock(scope: !299, file: !1, line: 101, column: 48)
!303 = !DILocation(line: 103, column: 13, scope: !302)
!304 = !DILocation(line: 105, column: 5, scope: !263)
!305 = !DILocation(line: 86, column: 32, scope: !257)
!306 = !DILocation(line: 86, column: 5, scope: !257)
!307 = distinct !{!307, !308}
!308 = !DILocation(line: 86, column: 5, scope: !9)
!309 = !DILocation(line: 110, column: 17, scope: !9)
!310 = !DILocation(line: 110, column: 9, scope: !9)
!311 = !DILocation(line: 110, column: 7, scope: !9)
!312 = !DILocation(line: 111, column: 10, scope: !313)
!313 = distinct !DILexicalBlock(scope: !9, file: !1, line: 111, column: 9)
!314 = !DILocation(line: 111, column: 9, scope: !9)
!315 = !DILocation(line: 112, column: 9, scope: !313)
!316 = !DILocation(line: 114, column: 14, scope: !9)
!317 = !DILocation(line: 114, column: 33, scope: !9)
!318 = !DILocation(line: 114, column: 38, scope: !9)
!319 = !DILocation(line: 114, column: 17, scope: !9)
!320 = !DILocation(line: 114, column: 5, scope: !9)
!321 = !DILocation(line: 115, column: 11, scope: !9)
!322 = !DILocation(line: 116, column: 5, scope: !9)
!323 = !DILocation(line: 117, column: 20, scope: !9)
!324 = !DILocation(line: 117, column: 23, scope: !9)
!325 = !DILocation(line: 117, column: 11, scope: !9)
!326 = !DILocation(line: 117, column: 9, scope: !9)
!327 = !DILocation(line: 118, column: 18, scope: !9)
!328 = !DILocation(line: 118, column: 5, scope: !9)
!329 = !DILocation(line: 120, column: 10, scope: !330)
!330 = distinct !DILexicalBlock(scope: !9, file: !1, line: 120, column: 9)
!331 = !DILocation(line: 120, column: 9, scope: !9)
!332 = !DILocation(line: 121, column: 9, scope: !330)
!333 = !DILocation(line: 124, column: 12, scope: !334)
!334 = distinct !DILexicalBlock(scope: !9, file: !1, line: 124, column: 5)
!335 = !DILocation(line: 124, column: 10, scope: !334)
!336 = !DILocation(line: 124, column: 17, scope: !337)
!337 = distinct !DILexicalBlock(scope: !334, file: !1, line: 124, column: 5)
!338 = !DILocation(line: 124, column: 21, scope: !337)
!339 = !DILocation(line: 124, column: 19, scope: !337)
!340 = !DILocation(line: 124, column: 5, scope: !334)
!341 = !DILocation(line: 125, column: 21, scope: !342)
!342 = distinct !DILexicalBlock(scope: !337, file: !1, line: 124, column: 32)
!343 = !DILocation(line: 125, column: 13, scope: !342)
!344 = !DILocation(line: 125, column: 11, scope: !342)
!345 = !DILocation(line: 126, column: 14, scope: !346)
!346 = distinct !DILexicalBlock(scope: !342, file: !1, line: 126, column: 13)
!347 = !DILocation(line: 126, column: 13, scope: !342)
!348 = !DILocation(line: 127, column: 13, scope: !349)
!349 = distinct !DILexicalBlock(scope: !346, file: !1, line: 126, column: 70)
!350 = !DILocation(line: 128, column: 13, scope: !349)
!351 = !DILocation(line: 130, column: 18, scope: !342)
!352 = !DILocation(line: 130, column: 37, scope: !342)
!353 = !DILocation(line: 130, column: 42, scope: !342)
!354 = !DILocation(line: 130, column: 21, scope: !342)
!355 = !DILocation(line: 130, column: 9, scope: !342)
!356 = !DILocation(line: 131, column: 9, scope: !342)
!357 = !DILocation(line: 132, column: 23, scope: !342)
!358 = !DILocation(line: 132, column: 19, scope: !342)
!359 = !DILocation(line: 132, column: 18, scope: !342)
!360 = !DILocation(line: 132, column: 13, scope: !342)
!361 = !DILocation(line: 132, column: 9, scope: !342)
!362 = !DILocation(line: 132, column: 16, scope: !342)
!363 = !DILocation(line: 133, column: 24, scope: !342)
!364 = !DILocation(line: 133, column: 27, scope: !342)
!365 = !DILocation(line: 133, column: 32, scope: !342)
!366 = !DILocation(line: 133, column: 15, scope: !342)
!367 = !DILocation(line: 133, column: 13, scope: !342)
!368 = !DILocation(line: 135, column: 14, scope: !369)
!369 = distinct !DILexicalBlock(scope: !342, file: !1, line: 135, column: 13)
!370 = !DILocation(line: 135, column: 13, scope: !342)
!371 = !DILocation(line: 136, column: 13, scope: !372)
!372 = distinct !DILexicalBlock(scope: !369, file: !1, line: 135, column: 61)
!373 = !DILocation(line: 137, column: 13, scope: !372)
!374 = !DILocation(line: 139, column: 25, scope: !342)
!375 = !DILocation(line: 139, column: 28, scope: !342)
!376 = !DILocation(line: 139, column: 34, scope: !342)
!377 = !DILocation(line: 139, column: 32, scope: !342)
!378 = !DILocation(line: 139, column: 53, scope: !342)
!379 = !DILocation(line: 139, column: 51, scope: !342)
!380 = !DILocation(line: 139, column: 39, scope: !342)
!381 = !DILocation(line: 139, column: 16, scope: !342)
!382 = !DILocation(line: 139, column: 13, scope: !342)
!383 = !DILocation(line: 140, column: 22, scope: !342)
!384 = !DILocation(line: 140, column: 9, scope: !342)
!385 = !DILocation(line: 142, column: 14, scope: !386)
!386 = distinct !DILexicalBlock(scope: !342, file: !1, line: 142, column: 13)
!387 = !DILocation(line: 142, column: 13, scope: !342)
!388 = !DILocation(line: 143, column: 13, scope: !389)
!389 = distinct !DILexicalBlock(scope: !386, file: !1, line: 142, column: 53)
!390 = !DILocation(line: 144, column: 13, scope: !389)
!391 = !DILocation(line: 146, column: 5, scope: !342)
!392 = !DILocation(line: 124, column: 28, scope: !337)
!393 = !DILocation(line: 124, column: 5, scope: !337)
!394 = distinct !{!394, !395}
!395 = !DILocation(line: 124, column: 5, scope: !9)
!396 = !DILocation(line: 149, column: 12, scope: !397)
!397 = distinct !DILexicalBlock(scope: !9, file: !1, line: 149, column: 5)
!398 = !DILocation(line: 149, column: 10, scope: !397)
!399 = !DILocation(line: 149, column: 17, scope: !400)
!400 = distinct !DILexicalBlock(scope: !397, file: !1, line: 149, column: 5)
!401 = !DILocation(line: 149, column: 21, scope: !400)
!402 = !DILocation(line: 149, column: 26, scope: !400)
!403 = !DILocation(line: 149, column: 19, scope: !400)
!404 = !DILocation(line: 149, column: 5, scope: !397)
!405 = !DILocalVariable(name: "delta", scope: !406, file: !1, line: 150, type: !5)
!406 = distinct !DILexicalBlock(scope: !400, file: !1, line: 149, column: 36)
!407 = !DILocation(line: 150, column: 13, scope: !406)
!408 = !DILocation(line: 152, column: 21, scope: !406)
!409 = !DILocation(line: 152, column: 13, scope: !406)
!410 = !DILocation(line: 152, column: 11, scope: !406)
!411 = !DILocation(line: 153, column: 14, scope: !412)
!412 = distinct !DILexicalBlock(scope: !406, file: !1, line: 153, column: 13)
!413 = !DILocation(line: 153, column: 13, scope: !406)
!414 = !DILocation(line: 154, column: 13, scope: !415)
!415 = distinct !DILexicalBlock(scope: !412, file: !1, line: 153, column: 70)
!416 = !DILocation(line: 155, column: 13, scope: !415)
!417 = !DILocation(line: 157, column: 18, scope: !406)
!418 = !DILocation(line: 157, column: 37, scope: !406)
!419 = !DILocation(line: 157, column: 42, scope: !406)
!420 = !DILocation(line: 157, column: 21, scope: !406)
!421 = !DILocation(line: 157, column: 9, scope: !406)
!422 = !DILocation(line: 158, column: 9, scope: !406)
!423 = !DILocation(line: 159, column: 18, scope: !424)
!424 = distinct !DILexicalBlock(scope: !406, file: !1, line: 159, column: 9)
!425 = !DILocation(line: 159, column: 14, scope: !424)
!426 = !DILocation(line: 159, column: 41, scope: !427)
!427 = distinct !DILexicalBlock(scope: !424, file: !1, line: 159, column: 9)
!428 = !DILocation(line: 159, column: 44, scope: !427)
!429 = !DILocation(line: 159, column: 50, scope: !427)
!430 = !DILocation(line: 159, column: 48, scope: !427)
!431 = !DILocation(line: 159, column: 55, scope: !427)
!432 = !DILocation(line: 159, column: 32, scope: !427)
!433 = !DILocation(line: 159, column: 30, scope: !427)
!434 = !DILocation(line: 159, column: 9, scope: !424)
!435 = !DILocation(line: 160, column: 20, scope: !436)
!436 = distinct !DILexicalBlock(scope: !427, file: !1, line: 159, column: 62)
!437 = !DILocation(line: 160, column: 17, scope: !436)
!438 = !DILocation(line: 159, column: 9, scope: !427)
!439 = distinct !{!439, !440}
!440 = !DILocation(line: 159, column: 9, scope: !406)
!441 = !DILocation(line: 162, column: 22, scope: !406)
!442 = !DILocation(line: 162, column: 9, scope: !406)
!443 = !DILocation(line: 164, column: 14, scope: !444)
!444 = distinct !DILexicalBlock(scope: !406, file: !1, line: 164, column: 13)
!445 = !DILocation(line: 164, column: 13, scope: !406)
!446 = !DILocation(line: 165, column: 13, scope: !447)
!447 = distinct !DILexicalBlock(scope: !444, file: !1, line: 164, column: 53)
!448 = !DILocation(line: 166, column: 13, scope: !447)
!449 = !DILocation(line: 168, column: 5, scope: !406)
!450 = !DILocation(line: 149, column: 32, scope: !400)
!451 = !DILocation(line: 149, column: 5, scope: !400)
!452 = distinct !{!452, !453}
!453 = !DILocation(line: 149, column: 5, scope: !9)
!454 = !DILocation(line: 170, column: 5, scope: !9)
!455 = !DILocation(line: 171, column: 1, scope: !9)
