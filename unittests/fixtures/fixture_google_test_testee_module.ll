@.str = private unnamed_addr constant [6 x i8] c"Hello\00", align 1 
@.str.1 = private unnamed_addr constant [6 x i8] c"world\00", align 1 

%"class.testing::internal::TestFactoryImpl" = type opaque 
%"class.testing::internal::TestFactoryBase" = type opaque 

%"class.testing::TestInfo" = type opaque 

%class.Hello_world_Test = type opaque 

@_ZN14Hello_world_Test10test_info_E = global %"class.testing::TestInfo"* null, align 8 

define void @__cxx_global_var_init() { 
entry: 
  %call = call i8* @_ZN7testing8internal13GetTestTypeIdEv() 
  %call1 = call i8* @_Znwm(i64 8) 
  %0 = bitcast i8* %call1 to %"class.testing::internal::TestFactoryImpl"* 
  call void @_ZN7testing8internal15TestFactoryImplI14Hello_sup_TestEC1Ev(%"class.testing::internal::TestFactoryImpl"* %0) 
  %1 = bitcast %"class.testing::internal::TestFactoryImpl"* %0 to %"class.testing::internal::TestFactoryBase"*  
  %call2 = call %"class.testing::TestInfo"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i8* null, i8* null, i8* %call, void ()* @_ZN7testing4Test13SetUpTestCaseEv, void ()* @_ZN7testing4Test16TearDownTestCaseEv, %"class.testing::internal::TestFactoryBase"* %1) 
  store %"class.testing::TestInfo"* %call2, %"class.testing::TestInfo"** @_ZN14Hello_world_Test10test_info_E, align 8 
  ret void 
}

declare void @_ZN7testing4Test13SetUpTestCaseEv() 
declare void @_ZN7testing4Test16TearDownTestCaseEv() 
declare i8* @_ZN7testing8internal13GetTestTypeIdEv() 
declare void @_ZN7testing8internal15TestFactoryImplI14Hello_sup_TestEC1Ev(%"class.testing::internal::TestFactoryImpl"* %this) 
declare %"class.testing::TestInfo"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8*, i8*, i8*, i8*, i8*, void ()*, void ()*, %"class.testing::internal::TestFactoryBase"*) 
declare i8* @_Znwm(i64) 

declare void @_ZN16Hello_world_Test8TestBodyEv(%class.Hello_world_Test* %this)